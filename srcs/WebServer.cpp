#include <fcntl.h>
#include <memory>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include "../includes/WebServer.hpp"
#define  PORT 8080

WebServer::WebServer(const std::string& config)
	: mConfig(config)
	, mPort(PORT)
	, mListenFd(-1)
	, mKq(-1)
{}

WebServer&	WebServer::operator=(const WebServer& rhs)
{
	if (this == &rhs)
		return (*this);
	mConfig = rhs.mConfig;
	mPort = PORT;
	mListenFd = -1;
	mKq = -1;
	return (*this);
}

void	WebServer::SetTCP(void)
{
	mListenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (mListenFd == -1)
		throw std::runtime_error("socket() error");

	const int REUSE_ADDR_ON = 1;
	if (setsockopt(mListenFd, SOL_SOCKET, SO_REUSEADDR, &REUSE_ADDR_ON, sizeof(REUSE_ADDR_ON)))
		throw std::runtime_error("setsockopt() error");

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(mPort);

	if (bind(mListenFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("bind() error");

	if (listen(mListenFd, SOMAXCONN) < 0)
		throw std::runtime_error("listen() error");

	int flags = fcntl(mListenFd, F_GETFL, 0);
	if (fcntl(mListenFd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl() error");

	mKq = kqueue();
	if (mKq < 0)
		throw std::runtime_error("kqueue() error");

	changeEvents(mListenFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	if (kevent(mKq, &mChangeList[0], mChangeList.size(), mEventList, 8, NULL) == -1)
		throw std::runtime_error("kevent() error");
}

void	WebServer::RunTCP(void)
{
	int newEvents;
	struct kevent* currEvent;

	while (1)
	{
		newEvents = kevent(mKq, &mChangeList[0], mChangeList.size(), mEventList, 8, NULL);
		if (newEvents == -1)
			throw std::runtime_error("kevent() error\n");
		mChangeList.clear();
		for (int i = 0; i < newEvents; ++i)
		{
			currEvent = &mEventList[i];
			if (currEvent->flags & EV_ERROR)
			{
				if (currEvent->ident == static_cast<uintptr_t>(mListenFd))
					throw std::runtime_error("server socket error");
				else
				{
					disconnectClient(currEvent->ident);
					throw std::runtime_error("Client connection error");
				}
			}
			else if (currEvent->filter == EVFILT_READ)
			{
				if (currEvent->ident == static_cast<uintptr_t>(mListenFd))
				{
					int clientSocket = accept(mListenFd, NULL, NULL);
					if (clientSocket == -1)
						throw std::runtime_error("accept() error");
					fcntl(clientSocket, F_SETFL, O_NONBLOCK);
					mClient[clientSocket] = "";
					changeEvents(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					changeEvents(clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
				}
				else if (mClient.find(currEvent->ident) != mClient.end())
				{
					// TODO: 2048을 넘어가는 HTTP Request는 어떻게 처리를 할 것인가?
					char buf[2048];
					int n = read(currEvent->ident, buf, sizeof(buf));
					if (n < 0)
						throw std::runtime_error("read() error");
					if (n == 0)
						disconnectClient(currEvent->ident);
					else
					{
						buf[n] = '\0';
						// TODO: mClient[currEvent->ident] += Http.makeResponse(buf);
						mClient[currEvent->ident] += buf; //temp
						printf("%s\n", buf); // HTTP Request 출력
					}
				}
			}
			else if (currEvent->filter == EVFILT_WRITE)
			{
				std::map<int, std::string>::iterator it = mClient.find(currEvent->ident);
				if (it != mClient.end())
				{
					if (mClient[currEvent->ident] != "")
					{
						// TODO;
						// Read event 발생했을 때 Http.makeResponse(buf); 의 결과값을 write 해야 함
						std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nHello, World!"; // temp
						if (write(currEvent->ident, httpResponse.c_str(), httpResponse.size()) == -1)
						{
							disconnectClient(currEvent->ident);
							throw std::runtime_error("write() error");
						}
						else
							mClient[currEvent->ident].clear();
					}
				}
			}
		}
	}
}

void	WebServer::changeEvents(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent temp_event;

	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	mChangeList.push_back(temp_event);
}

void	WebServer::disconnectClient(int clientSocket)
{
	close(clientSocket);
	mClient.erase(clientSocket);
}
