#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <exception>
#include <cstdio>
#include "WebServ.hpp"
#include "HttpHandler.hpp"
#include "Response.hpp"

WebServ::WebServ(const std::set<int>& portList, const std::vector<std::string>& envList)
{
	mEnvList = envList;
	createServerSocket(portList);
	setKqueue();
	runKqueueLoop();
}

WebServ::~WebServ()
{}

void WebServ::createServerSocket(const std::set<int>& portList)
{
	std::set<int>::iterator iter = portList.begin();
	for (; iter != portList.end(); ++iter)
	{
		int listenFd = socket(AF_INET, SOCK_STREAM, 0);
		if (listenFd == -1)
			throw std::runtime_error("socket error");

		const int REUSE_ADDR_ON = 1;
		if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &REUSE_ADDR_ON, sizeof(REUSE_ADDR_ON)))
			throw std::runtime_error("setsockopt error");

		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(*iter);

		if (bind(listenFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
			throw std::runtime_error("bind error");
		if (listen(listenFd, SOMAXCONN) < 0)
			throw std::runtime_error("listen error");
		if (fcntl(listenFd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0)
			throw std::runtime_error("fcntl error");
		mServSockList.push_back(listenFd);
		mServSockPortMap[listenFd] = *iter;
	}
}

void	WebServ::setKqueue(void)
{
	mKq = kqueue();
	if (mKq < 0)
		throw std::runtime_error("kqueue error");

	std::vector<int>::iterator iter = mServSockList.begin();
	for (; iter != mServSockList.end(); iter++)
		addEvents(*iter, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	// mChangeList.clear();
}

void	WebServ::addEvents(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent newEvent;

	EV_SET(&newEvent, ident, filter, flags, fflags, data, udata);
	if (kevent(mKq, &newEvent, 1, NULL, 0, NULL) == -1)
		throw std::runtime_error("kevent error");
}

bool	WebServ::isFatalKeventError(void)
{
	std::cerr << errno << std::endl;
	switch (errno)
	{
		case ENOENT:
		case EBADF:
		case EINTR:
		case ESRCH:
		case EACCES:
		case EINVAL:
			return (false);
		case EFAULT:
		case ENOMEM:
			return (true);
		default:
			break;
	}
	return (false);
}

void	WebServ::runKqueueLoop(void)
{
	int numOfNewEvent;
	struct kevent* currEvent;

	// Not pendding kevent
	struct timespec timeout;
	timeout.tv_nsec = 0;
	timeout.tv_sec = 0;
	while (1)
	{
		try
		{
			numOfNewEvent = kevent(mKq, NULL, 0, mEventList, 30, &timeout);
			if (numOfNewEvent == -1)
			{
				if (isFatalKeventError())
					break ;
				else
					continue ;
			}
			for (int i = 0; i < numOfNewEvent; i++)
			{
				try
				{
					currEvent = &mEventList[i];
					if (currEvent->fflags & EV_ERROR)
						throw std::runtime_error("kevent error1");
					else if (currEvent->filter == EVFILT_READ)
					{
						if (std::find(mServSockList.begin(), mServSockList.end(), currEvent->ident) != mServSockList.end())
							acceptNewClientSocket(currEvent);
						else if (mRequestMap.find(currEvent->ident) != mRequestMap.end())
							processHttpRequest(currEvent);
						else if (mCGIPipeMap.find(currEvent->ident) != mCGIPipeMap.end())
							sendPipeData(currEvent);
					}
					else if (currEvent->filter == EVFILT_WRITE)
					{
						std::cout << "Write event occured!" << std::endl; // debug
						if (mResponseMap.find(currEvent->ident) != mResponseMap.end())
							writeHttpResponse(currEvent);
						else if (mCGIPostPipeMap.find(currEvent->ident) != mCGIPostPipeMap.end())
							writeToCGIPipe(currEvent);
					}
					else if (currEvent->filter == EVFILT_PROC)
						waitCGIProc(currEvent);
					else if (currEvent->filter == EVFILT_TIMER)
						handleTimeOut(currEvent);
				}
				catch(const std::exception& e)
				{
					perror(e.what());
					continue;
				}
			}
		}
		catch(const std::exception& e)
		{
			perror(e.what());
			continue;
		}
	}

}

// clientFD 기준으로 write event가 한 번도 안났으면 timeOut
// Response 순회하면서
// CGI(pipe, pid) 이벤트 제거
void	WebServ::handleTimeOut(struct kevent* currEvent)
{
	int clientFD = currEvent->ident;

	addEvents(clientFD, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	std::deque<Response>::iterator iter = mResponseMap[clientFD].begin();
	for (; iter != mResponseMap[clientFD].end(); ++iter)
	{
		if (iter->IsCGI())
		{
			int pipeFD = mCGIClientMap[clientFD].first;
			int pid = mCGIPipeMap[pipeFD].second;
			close(pipeFD); // pipe event 삭제
			eraseCGIMaps(pid, clientFD, pipeFD);
			addEvents(pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL); // pid 이벤트 삭제
		}
		mResponseMap[clientFD].front().SetStatusOf(504);
		addEvents(clientFD, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL); // Error page return
	}
}

void	WebServ::waitCGIProc(struct kevent* currEvent)
{
	int pid = currEvent->ident;
	int pipeFD = mCGIPidMap[pid].second;
	int clientFD = mCGIPipeMap[pipeFD].second;
	int status;
	Response* response = mCGIPidMap[pid].first;
	if (currEvent->fflags & NOTE_EXIT)
	{
		waitpid(pid, &status, 0);
		if (status != 0)
			response->SetStatusOf(502);
		else
		{
			response->GenCGIBody();
			addEvents(clientFD, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
		}
		close(pipeFD);
		eraseCGIMaps(pid, clientFD, pipeFD);
	}
}


void	WebServ::acceptNewClientSocket(struct kevent* currEvent)
{
	// std::cout << "acceptNewClientSocket" << std::endl; // debug
	int servSocket = currEvent->ident;
	int clientSocket = accept(servSocket, NULL, NULL);
	if (clientSocket == -1)
		throw std::runtime_error("accept() error");
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1)
		throw std::runtime_error("fcntl() error");
	addEvents(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	mRequestMap.insert(std::make_pair(clientSocket, HttpHandler(mServSockPortMap[servSocket])));
	mResponseMap.insert(std::make_pair(clientSocket, std::deque<Response>()));
}

void	WebServ::processHttpRequest(struct kevent* currEvent)
{
	// std::cout << "processHttpRequest" << std::endl; // debug
	int clientFD = currEvent->ident;

	if (currEvent->flags & EV_EOF) // 클라이언트 쪽에서 소켓을 닫음 (관련된 모든 리소스를 삭제, 어쩌피 에러도 못 받음)
	{
		close(clientFD); // EVFILT_READ, EVFILT_WRITE 삭제
		mRequestMap.erase(clientFD);
		mResponseMap.erase(clientFD);
		return;
	}
	std::string httpRequest = readFDData(clientFD);
	std::cout << httpRequest; // debug
	addEvents(clientFD, EVFILT_TIMER, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 30000, NULL); // 30초 타임아웃 (write event가 발생하면 timeout event를 삭제해줘야 함)
	mTimerMap[clientFD] = true;
	// TODO: ConfigHandler::GetResponseOf 메서드와 중복 책임. => 하나로 병합 또는 한 쪽 삭제 요망
	// Request 객체로부터 RequestList를 받음
	std::deque<Response> responseList = mRequestMap[clientFD].MakeResponseOf(httpRequest);
	std::deque<Response>::iterator responseIt = responseList.begin();
	for(; responseIt != responseList.end(); ++responseIt)
	{
		std::cout << "looping response list..." << std::endl; //debug
		mResponseMap[clientFD].push_back(*responseIt);
		if (!responseIt->IsCGI())
		{
			// std::cout << "Not CGI, set new Write Event" << std::endl; // debug
			addEvents(clientFD, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
		}
		else // Get인지 Post인지 확인을 해야 함
			processCGI(mResponseMap[clientFD].back(), clientFD);
	}
}

void	WebServ::processCGI(Response& response, int clientFD)
{
	int readFD[2];
	int writeFD[2];

	if (pipe(readFD) == -1 || pipe(writeFD))
		throw std::runtime_error("pipe error");
	if (fcntl(readFD[0], F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1 || fcntl(writeFD[1], F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1)
		throw std::runtime_error("fcntl() error");
	addEvents(readFD[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	if (response.GetRequestBody().size() != 0) // post
	{
		addEvents(writeFD[1], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
		mCGIPostPipeMap.insert(std::make_pair(writeFD[1], std::make_pair(&response, 0)));
	}
	pid_t pid = fork();
	if (pid < 0)
		throw std::runtime_error("fork error");
	else if (pid == 0) // child
	{
		close(readFD[0]);
		close(writeFD[1]);
		if (dup2(readFD[1], STDOUT_FILENO) == -1)
		{
			perror("dup2 error for stdout");
			exit(EXIT_FAILURE);
		}
		if (dup2(writeFD[0], STDIN_FILENO) == -1)
		{
			perror("dup2 error for stdin");
			exit(EXIT_FAILURE);
		}
		char *const *argv = makeArgvList(response.GetCGIPath(), response.GetABSPath()); // 인자는 또 없나?
		char *const *envp = makeCGIEnvList(response);
		if (execve(argv[0], argv, envp) == -1)
		{
			perror("execve failed");
			exit(EXIT_FAILURE);
		}
	}
	else // parent
	{
		close(readFD[1]);
		addEvents(pid, EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT, 0, NULL);
		mCGIClientMap[clientFD] = std::make_pair(readFD[0], pid);
		mCGIPipeMap[readFD[0]] = std::make_pair(&response, clientFD);
		mCGIPidMap[pid] = std::make_pair(&response, readFD[0]);
	}
}


char *const *WebServ::makeArgvList(const std::string& CGIPath, const std::string& ABSPath)
{
	char **argv = new char*[3]; // v + ABSPath;
	argv[0] = new char[CGIPath.size() + 1];
	std::strcpy(argv[0], CGIPath.c_str());
	argv[1] = new char[ABSPath.size() + 1];
	std::strcpy(argv[1], ABSPath.c_str());
	argv[2] = NULL;
	return (argv);
}

char *const *WebServ::makeCGIEnvList(const Response& response)
{
	std::map<std::string, std::string> params = response.GetParams();
	for (std::map<std::string, std::string>::iterator iter = params.begin(); iter != params.end(); ++iter)
	{
		std::string s = iter->first + "=" + iter->second;
		mEnvList.push_back(s);
	}
	char **envp = new char*[mEnvList.size() + 1];
	for (size_t i = 0; i < mEnvList.size(); ++i)
	{
		envp[i] = new char[mEnvList[i].length() + 1];
		std::strcpy(envp[i], mEnvList[i].c_str());
	}
	envp[mEnvList.size()] = NULL;
	return (envp);
}

void	WebServ::sendPipeData(struct kevent* currEvent)
{
	int pipeFD = currEvent->ident;
	mCGIPipeMap[pipeFD].first->AppendCGIBody(readFDData(pipeFD));
}

void	WebServ::writeToCGIPipe(struct kevent* currEvent)
{
	int pipeFD = currEvent->ident;
	Response &response = *(mCGIPostPipeMap[pipeFD].first);
	size_t pos = mCGIPostPipeMap[pipeFD].second;
	ssize_t written = write(pipeFD, response.GetRequestBody().c_str() + pos, response.GetRequestBody().size() - pos);
	if (written == -1)
		throw std::runtime_error("write error");
	mCGIPostPipeMap[pipeFD].second += written;
	if (mCGIPostPipeMap[pipeFD].second == response.GetRequestBody().size())
	{
		close(pipeFD);
		mCGIPostPipeMap.erase(pipeFD);
	}
}

void	WebServ::writeHttpResponse(struct kevent* currEvent)
{
	// std::cout << "writeHttpResponse" << std::endl; // debug
	int clientFD = currEvent->ident;
	Response &response = mResponseMap[clientFD].front();

	if (currEvent->fflags & EV_EOF /* || response.IsConnectionStop() == true */) // TODO: connectionStop;
	{
		close(clientFD);
		eraseHttpMaps(clientFD);
		return ;
	}
	response.WriteResponseHeaderTo(clientFD);
	response.WriteResponseBodyTo(clientFD);
	if (mTimerMap[clientFD])
	{
		addEvents(clientFD, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
		mTimerMap[clientFD] = false;
	}
	mResponseMap[clientFD].pop_front();
	if (mResponseMap[clientFD].size() == 0)
		addEvents(clientFD, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
}

std::string	WebServ::readFDData(int clientFD)
{
	char buf[2048];
	int n = read(clientFD, buf, sizeof(buf)-1);
	if (n < 0)
	{
		close(clientFD);
		throw std::runtime_error("read error");
	}
	buf[n] = 0;
	return (std::string(buf));
}

void WebServ::eraseCGIMaps(int pid, int clientFD, int pipeFD)
{
	if (mCGIPidMap.find(pid) != mCGIPidMap.end())
		mCGIPidMap.erase(pid);
	if (mCGIClientMap.find(clientFD) != mCGIClientMap.end())
		mCGIClientMap.erase(clientFD);
	if (mCGIPipeMap.find(pipeFD) != mCGIPipeMap.end())
		mCGIPipeMap.erase(pipeFD);
}

void WebServ::eraseHttpMaps(int clientFD)
{
	if (mRequestMap.find(clientFD) != mRequestMap.end())
		mRequestMap.erase(clientFD);
	if (mResponseMap.find(clientFD) != mResponseMap.end())
		mResponseMap.erase(clientFD);
}


