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
				throw std::runtime_error("kevent error");
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
						else if (mCGIBodyBuf.find(currEvent->ident) != mCGIBodyBuf.end())
							sendCGIResource(currEvent);
					}
					else if (currEvent->filter == EVFILT_WRITE)
						;
					else if (currEvent->filter == EVFILT_PROC)
						;
					else if (currEvent->filter == EVFILT_TIMER)
						;
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


void	WebServ::acceptNewClientSocket(struct kevent* currEvent)
{
	// TODO: cliAddr 정보 활용할 상황이 생길 수 있음
	struct sockaddr cliAddr;
	socklen_t cliLen = sizeof(cliAddr);
	int servSocket = currEvent->ident;
	int clientSocket = accept(servSocket, &cliAddr, &cliLen);
	if (clientSocket == -1)
		throw std::runtime_error("accept() error");
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1)
		throw std::runtime_error("fcntl() error");
	addEvents(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	mRequestMap[clientSocket] = Request();
}

void	WebServ::processHttpRequest(struct kevent* currEvent)
{
	int clientFD = currEvent->ident;

	if (currEvent->flags & EV_EOF)
		addEvents(clientFD, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	std::string httpRequest = readHttpRequest(clientFD);

	// Request 객체로부터 RequestList를 받음
	// std::vector<Request> RequestList = mRequestMap[clientFD].GetResponseOf(httpRequest);

	// Response queue를 받음
	// mResponseMap[clientFD] = ConfigHandler::GetResponseOf(RequestList); // Response 객체 생성

	// 이미 객체 생성되면서 만들 수 있는 데이터는 다 만듬
	// Response를 순회하면서 Static인지 CGI인지 구분
	std::deque<Response>::iterator iter = mResponseMap[clientFD].begin();
	for (; iter != mResponseMap[clientFD].end(); ++iter)
	{
		// addEvent(clientFD, EV_WRITE);
		//
		// if (mResponseMap[clientFD].IsCGI())
		// 	pipe()
		// 	addEvent(pipeFD, EVFILT_READ);
		// 	mCGIBodyBuf[pipeFD] = iter; pipeFD와 Buf를 매핑
		// 	fork()
		// 	addEvent(ChildPID, EVFILT_PROC);
	}
}

void	WebServ::sendCGIResource(struct kevent* currEvent)
{

}

std::string	WebServ::readHttpRequest(int clientFD)
{
	char buf[2048];
	int n = read(clientFD, buf, sizeof(buf));
	if (n < 0)
		throw std::runtime_error("read error");
	buf[n] = 0;
	return (std::string(buf));
}
