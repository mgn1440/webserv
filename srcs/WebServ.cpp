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
#include <signal.h>
#include "WebServ.hpp"
#include "HttpHandler.hpp"
#include "Response.hpp"
#include "STLUtils.hpp"

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
}

void	WebServ::addEvents(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent newEvent;

	EV_SET(&newEvent, ident, filter, flags, fflags, data, udata);
	// std::cout << "ident: " << ident << ", filter: " << filter << ", flags: " << flags << std::endl; 
	if (kevent(mKq, &newEvent, 1, NULL, 0, NULL) == -1)
	{
		if (errno == EBADF)
		{
			std::cout << "01" << std::endl;
			eraseClientMaps(ident);
			if (mTimerMap[ident])
			{
				addEvents(ident, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
				mTimerMap[ident] = false;
			}
		}
		else
			perror("addEventError: ");
	}
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

	struct timespec timeout;
	timeout.tv_nsec = 0;
	timeout.tv_sec = 0;
	while (1)
	{
		try
		{
			numOfNewEvent = kevent(mKq, NULL, 0, mEventList, KQ_EVENT_SIZE, &timeout);
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
						throw std::runtime_error("kevent error");
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

void	WebServ::handleTimeOut(struct kevent* currEvent)
{
	int clientFD = currEvent->ident;

	std::deque<Response>::iterator iter = mResponseMap[clientFD].begin();
	for (; iter != mResponseMap[clientFD].end(); ++iter)
	{
		if (iter->IsCGI())
		{
			if (mCGIClientMap.find(clientFD) == mCGIClientMap.end())
				continue;
			int pipeFD = mCGIClientMap[clientFD].first;
			pid_t pid = mCGIClientMap[clientFD].second;
			close(pipeFD); // pipe event 삭제
			std::cout << "1" << std::endl; // debug
			mCGIPostPipeMap.erase(pipeFD); // suro1
			eraseCGIMaps(pid, clientFD, pipeFD);
			addEvents(pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL); // pid 이벤트 삭제
			if (kill(pid, SIGTERM) == -1)
				throw std::runtime_error("kill");
		}
		iter->SetStatusOf(504, "");
	}
	addEvents(clientFD, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
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
		if (!WIFEXITED(status) || WEXITSTATUS(status))
			response->SetStatusOf(502, "");
		else
			response->GenCGIBody();
		addEvents(clientFD, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
		close(pipeFD);
		std::cout << "2" << std::endl; // debug
		mCGIPostPipeMap.erase(pipeFD); // suro1
		eraseCGIMaps(pid, clientFD, pipeFD);
	}
}

void	WebServ::acceptNewClientSocket(struct kevent* currEvent)
{
	struct linger _linger;
	_linger.l_onoff = 1;
	_linger.l_linger = 0;
	int servSocket = currEvent->ident;
	int clientSocket = accept(servSocket, NULL, NULL);
	if (clientSocket == -1)
		throw std::runtime_error("accept error");
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1)
		throw std::runtime_error("fcntl error");
	addEvents(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	if (setsockopt(clientSocket, SOL_SOCKET, SO_LINGER, &_linger, sizeof(_linger)))
		throw std::runtime_error("setsockopt error");
	mRequestMap.insert(std::make_pair(clientSocket, HttpHandler(mServSockPortMap[servSocket])));
	mResponseMap.insert(std::make_pair(clientSocket, std::deque<Response>()));
}

void	WebServ::processHttpRequest(struct kevent* currEvent)
{
	int clientFD = currEvent->ident;;
	char buf[65535];
	int n = read(clientFD, buf, sizeof(buf));
	if (n == 0 && (currEvent->flags & EV_EOF))
	{
		close(clientFD);
		std::cout << "02" << std::endl;
		pid_t CGIPid = mResponseMap[clientFD].front().GetCGIPid();
		kill (CGIPid, SIGTERM); // 미완성
		int writePipe =mResponseMap[clientFD].front().GetWritePipeFd(); 
		mCGIPostPipeMap.erase(writePipe);
		close(writePipe);
		eraseClientMaps(clientFD); // suro 범인
		return ;
	}
	else if (n == -1) // 1. first request read return -1 is fatal error
		throw std::runtime_error("http request read error");
	addEvents(clientFD, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, 10000, NULL); // 10초 타임아웃 (write event가 발생하면 timeout event를 삭제해줘야 함)
	mTimerMap[clientFD] = true;
	std::deque<Response> responseList = mRequestMap[clientFD].MakeResponseOf(std::string(buf, n));
	std::deque<Response>::iterator responseIt = responseList.begin();
	for(; responseIt != responseList.end(); ++responseIt)
	{
		mResponseMap[clientFD].push_back(*responseIt);
		if (!responseIt->IsCGI())
			addEvents(clientFD, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
		else
			processCGI(mResponseMap[clientFD].back(), clientFD);
	}
}

void	WebServ::processCGI(Response& response, int clientFD)
{
	std::cout << "5" << std::endl; // debug
	int readFD[2];
	int writeFD[2];

	if (pipe(readFD) == -1 || pipe(writeFD) == -1)
		throw std::runtime_error("pipe error");
	if (fcntl(readFD[0], F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1 || fcntl(writeFD[1], F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1)
		throw std::runtime_error("fcntl error");
	addEvents(readFD[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
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
		if (response.GetRequestBody().size() != 0) // post
		{
			addEvents(writeFD[1], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
			mCGIPostPipeMap.insert(std::make_pair(writeFD[1], std::make_pair(&response, 0)));
		}
		else
			close(writeFD[1]);
		addEvents(pid, EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT, 0, NULL);
		response.SetCGIPid(writeFD[1]);
		close(readFD[1]);
		close(writeFD[0]);
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

char *const *WebServ::makeCGIEnvList(Response& response)
{
	std::map<std::string, std::string> params = response.GetParams();
	char **envp = new char*[mEnvList.size() + params.size() + 1];
	size_t i = 0;
	for (; i < mEnvList.size(); ++i)
	{
		envp[i] = new char[mEnvList[i].length() + 1];
		std::strcpy(envp[i], mEnvList[i].c_str());
	}
	for (std::map<std::string, std::string>::iterator iter = params.begin(); iter != params.end(); ++iter)
	{
		std::string s = iter->first + "=" + iter->second;
		envp[i] = new char[s.length() + 1];
		std::strcpy(envp[i], s.c_str());
		i ++;
	}
	envp[mEnvList.size() + params.size()] = NULL;
	return (envp);
}

void	WebServ::sendPipeData(struct kevent* currEvent)
{
	int pipeFD = currEvent->ident;
	char buf[65535];

	int n = read(pipeFD, buf, sizeof(buf));
	if (n == 0 && (currEvent->flags & EV_EOF))
	{
		close(pipeFD);
		std::cout << "4" << std::endl; // debug
		mCGIPostPipeMap.erase(pipeFD); // suro
		return ;
	}
	else if (n == -1)
		throw std::runtime_error("pipe read error");
	mCGIPipeMap[pipeFD].first->AppendCGIBody(std::string(buf, n));
}

void	WebServ::writeToCGIPipe(struct kevent* currEvent)
{
	int pipeFD = currEvent->ident;
	if (mCGIPostPipeMap.find(pipeFD) == mCGIPostPipeMap.end())
		return ;
	Response &response = *(mCGIPostPipeMap[pipeFD].first);
	size_t pos = mCGIPostPipeMap[pipeFD].second;
	size_t toWrite = response.GetRequestBody().size() - pos; // error: seg fault heap-use-after-free
	ssize_t written = write(pipeFD, response.GetRequestBody().c_str() + pos, toWrite);
	std::cout << "pipeFD = " << pipeFD << ", " << "written = " << written << std::endl;
	if ((written == 0 && (currEvent->fflags & EV_EOF)) || written == -1)
	{
		close(pipeFD);
		std::cout << "9" << std::endl; // debug
		mCGIPostPipeMap.erase(pipeFD);
		if (written == -1)
			perror("write error");
		return ;
	}
	mCGIPostPipeMap[pipeFD].second = pos + written;
	std::cout << "second = " << mCGIPostPipeMap[pipeFD].second  << ", " << "size = " << response.GetRequestBody().size() << std::endl; 
	if (mCGIPostPipeMap[pipeFD].second == response.GetRequestBody().size())
	{
		close(pipeFD);
		std::cout << "10" << std::endl; // debug
		mCGIPostPipeMap.erase(pipeFD);
	}
}

void	WebServ::writeHttpResponse(struct kevent* currEvent)
{
	std::cout << "11" << std::endl; // debug
	int clientFD = currEvent->ident;
	Response &response = mResponseMap[clientFD].front();

	response.CreateResponseHeader();
	if (response.WriteResponse(clientFD) == 0 && currEvent->fflags & EV_EOF)
	{
		close(clientFD);
		std::cout << "03" << std::endl;
		eraseClientMaps(clientFD);
		return ;
	}
	if (response.GetSendStatus() != SEND_ALL)
		return;
	mResponseMap[clientFD].pop_front();
	if (mResponseMap[clientFD].size() == 0)
	{
		addEvents(clientFD, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
		if (mTimerMap[clientFD])
		{
			addEvents(clientFD, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
			mTimerMap[clientFD] = false;
		}
	}
	// if (response.IsConnectionStop())
	// {
	// 	close(clientFD);
	// 	eraseClientMaps(clientFD);
	// }
}

void WebServ::eraseClientMaps(int clientFD)
{
	if (mRequestMap.find(clientFD) != mRequestMap.end())
		mRequestMap.erase(clientFD);
	if (mResponseMap.find(clientFD) != mResponseMap.end())
		mResponseMap.erase(clientFD);
	if (mCGIClientMap.find(clientFD) != mCGIClientMap.end())
	{
		int pipeFD = mCGIClientMap[clientFD].first;
		close(pipeFD); // suro1
		std::cout << "11" << std::endl; // debug
		mCGIPostPipeMap.erase(pipeFD); // suro1
		pid_t PID = mCGIClientMap[clientFD].second;
		if (mCGIPidMap.find(PID) != mCGIPidMap.end())
			mCGIPidMap.erase(PID);
		if (mCGIPipeMap.find(pipeFD) != mCGIPipeMap.end())
			mCGIPipeMap.erase(pipeFD);
		mCGIClientMap.erase(clientFD);
	}
}

void WebServ::eraseCGIMaps(int PID, int clientFD, int pipeFD)
{
	if (mCGIPidMap.find(PID) != mCGIPidMap.end())
		mCGIPidMap.erase(PID);
	if (mCGIClientMap.find(clientFD) != mCGIClientMap.end())
		mCGIClientMap.erase(clientFD);
	if (mCGIPipeMap.find(pipeFD) != mCGIPipeMap.end())
		mCGIPipeMap.erase(pipeFD);
	// close(pipeFD); // suro 1
}


