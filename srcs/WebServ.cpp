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
#include "WebServ.hpp"

WebServ::WebServ(std::vector<int> portList)
{
	createServerSocket(portList);
	setKqueue();
	runKqueueLoop();
}

void WebServ::createServerSocket(std::vector<int> portList)
{
	std::vector<int>::iterator iter = portList.begin();
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
	if (kevent(mKq, &mChangeList[0], mChangeList.size(), NULL, 0, NULL) == -1)
		throw std::runtime_error("kevent error");
	mChangeList.clear();
}

void	WebServ::addEvents(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent newEvent;

	EV_SET(&newEvent, ident, filter, flags, fflags, data, udata);
	mChangeList.push_back(newEvent);
}

bool	WebServ::handleKeventError(void)
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
						throw std::runtime_error("kevent error");
					else if (currEvent->filter == EVFILT_READ)
					{
						if (std::find(mServSockList.begin(), mServSockList.end(), currEvent->ident) != mServSockList.end())
							acceptNewClientSocket(currEvent);
						else if (mRequestMap.find(currEvent->ident) != mRequestMap.end())
							processHttpRequest(currEvent);
						else if (mPipeMap.find(currEvent->ident) != mPipeMap.end())
							sendCGIResource(currEvent);
					}
					else if (currEvent->filter == EVFILT_WRITE)
						writeHttpResponse(currEvent);
					else if (currEvent->filter == EVFILT_PROC)
						waitCGIProc(currEvent);
					else if (currEvent->filter == EVFILT_TIMER)
						handleTimeOut(currEvent);
				}
				catch(const std::exception& e)
				{
					std::cerr << e.what() << '\n';
					continue;
				}
			}
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
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

	std::deque<Response>::iterator iter = mResponseMap[clientFD].begin();
	for (; iter != mResponseMap[clientFD].end(); ++iter)
	{
		addEvents(clientFD, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		// if (iter->IsCGI())
		{
			int pipeFD = mClientFDMap[clientFD].first;
			int pid = mPipeMap[pipeFD].second;
			close(pipeFD); // pipe event 삭제
			mPipeMap.erase(pipeFD);
			mClientFDMap.erase(clientFD);
			mPidMap.erase(pid);
			addEvents(pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL); // pid 이벤트 삭제
		}
		// onErrorPage();
		addEvents(clientFD, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL); // Error page return
	}
}
void	WebServ::waitCGIProc(struct kevent* currEvent)
{
	int pid = currEvent->ident;
	int status;
	int pipeFD = mPidMap[pid].second;
	Response* response = mPidMap[pid].first;
	if (currEvent->fflags & NOTE_EXIT)
	{
		waitpid(pid, &status, 0);
		addEvents(pipeFD, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		if (!status)
			; // response->GetErrorPage(status); 비정상 종료일 때 어떻게 처리를 해야 하는거지
	}
}


void	WebServ::acceptNewClientSocket(struct kevent* currEvent)
{
	struct sockaddr *tmpAddr;
	socklen_t cliLen = sizeof(tmpAddr);
	int servSocket = currEvent->ident;
	int clientSocket = accept(servSocket, tmpAddr, &cliLen);
	if (clientSocket == -1)
		throw std::runtime_error("accept() error");
	struct sockaddr_in servAddr = *(struct sockaddr_in *)tmpAddr;
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1)
		throw std::runtime_error("fcntl() error");
	addEvents(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	// mRequestMap[clientSocket] = HttpRequest(servAddr.sin_port);
}

void	WebServ::processHttpRequest(struct kevent* currEvent)
{
	int clientFD = currEvent->ident;

	if (currEvent->flags & EV_EOF) // 클라이언트 쪽에서 소켓을 닫음 (관련된 모든 리소스를 삭제, 어쩌피 에러도 못 받음)
	{
		close(clientFD); // EVFILT_READ, EVFILT_WRITE 삭제
		throw std::runtime_error("client socket close");
	}
	std::string httpRequest = readFDData(clientFD);
	addEvents(clientFD, EVFILT_TIMER, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 30000, NULL); // 30초 타임아웃 (write event가 발생하면 timeout event를 삭제해줘야 함)

	// Request 객체로부터 RequestList를 받음
	// std::vector<Request> RequestList = mRequestMap[clientFD].GetResponseOf(httpRequest);

	// Response queue를 받음
	// mResponseMap[clientFD] = ConfigHandler::GetResponseOf(RequestList);

	// 이미 객체 생성되면서 만들 수 있는 데이터는 다 만듬
	// Response를 순회하면서 Static인지 CGI인지 구분
	std::deque<Response>::iterator iter = mResponseMap[clientFD].begin();
	for (; iter != mResponseMap[clientFD].end(); ++iter)
	{
		// addEvent(clientFD, EV_WRITE);
		//
		// if (mResponseMap[clientFD].IsCGI())
		//		pipe()
		//		addEvent(pipeFD, EVFILT_READ);
		//		mPipeMap[pipeFD] = std::make_pair(iter, clientFD); pipeFD와 Buf를 매핑
		//		fork()
		//		addEvent(ChildPID, EVFILT_PROC, ONE_SHOT);
		//		mClientFDMap[clientFD] = std::make_pair(pipeFD, pid);
		//		mPidMap[ChildPID] = std::make_pair(iter, pipeFD);
	}
}

void	WebServ::sendCGIResource(struct kevent* currEvent)
{
	int pipeFD = currEvent->ident;
	Response* CGIResponse = mPipeMap[pipeFD].first;
	int clientFD = mPipeMap[pipeFD].second;
	std::string CGIResource = readFDData(pipeFD);
	// CGIResponse->AppendCGIBody(CGIResource); // Response Buffer에 CGIResource를 모으는 중
	addEvents(clientFD, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, NULL); // EV_CLEAR를 사용하면 데이터 입력 발생할 때 한 번만 이벤트 발생함
}

void	WebServ::writeHttpResponse(struct kevent* currEvent)
{
	int clientFD = currEvent->ident;
	Response response = mResponseMap[clientFD].front();
	mResponseMap[clientFD].pop_front();

	if (currEvent->fflags & EV_EOF)
	{
		close(clientFD); // EVFILT_WRIT, EVFILT_READ, EVFILT_TIME 삭제
		// if (response.IsCGI())
		//	addEvents(PipeFD, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		//	addEvents(Pid, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
		//	mPipeMap.erase(mClientFDMap[clientFD].first);
		//	mClientFDMap.erase(clientFD);
		return ;
	}
	// std::string httpResponse = response.GetResponse();
	// if (write(clientFD, httpResponse.c_str(), httpResponse.size()) == -1)
	//		throw std::runtime_error("write error");
	// addEvent(clientFD, EVFILT_TIMEOUT, EV_DELETE, 0, 0, NULL);
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
