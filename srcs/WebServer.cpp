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
#include "../includes/Http.hpp"
#define  PORT 8080

WebServer::WebServer(const std::string& config)
	: mPort(PORT)
	, mListenFd(-1)
	, mKq(-1)
	, mConfigHandler(config)
{}

WebServer::~WebServer()
{
	std::map<int, Http*>::iterator it = mConnection.begin();
	for (; it != mConnection.end(); ++it)
		delete it->second;
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
			throw std::runtime_error("kevent() error");
		mChangeList.clear();
		for (int i = 0; i < newEvents; ++i)
		{
			currEvent = &mEventList[i];
			if (currEvent->flags & EV_ERROR)
				handleErrorEvent(currEvent->ident);
			else if (currEvent->filter == EVFILT_READ)
			{
				if (currEvent->ident == static_cast<uintptr_t>(mListenFd))
					acceptNewClientSocket();
				else if (mConnection.find(currEvent->ident) != mConnection.end())
					sendRequestToHttp(currEvent->ident);
			}
			else if (currEvent->filter == EVFILT_WRITE)
				writeResponseToClient(currEvent->ident);
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
	mConnection.erase(clientSocket);
	mClient.erase(clientSocket);
}

void	WebServer::acceptNewClientSocket(void)
{
	int clientSocket = accept(mListenFd, NULL, NULL);
	if (clientSocket == -1)
		throw std::runtime_error("accept() error");
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);
	changeEvents(clientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	changeEvents(clientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	mConnection[clientSocket] = new Http(mConfigHandler);
	mClient[clientSocket] = ""; // TODO : 응답 로직 완성된 후 삭제
}

void	WebServer::sendRequestToHttp(int clientFD)
{
	char buf[2048];
	int n = read(clientFD, buf, sizeof(buf));
	if (n < 0)
		throw std::runtime_error("read() error");
	if (n == 0)
		disconnectClient(clientFD);
	else
	{
		buf[n] = '\0';
		mClient[clientFD] += buf; // debug
		mConnection[clientFD]->ReciveRequestMessage(buf);
		std::cout << buf << std::endl; // HTTP Request 출력 debug
	}
}

void	WebServer::writeResponseToClient(int clientFD)
{
	std::map<int, Http*>::iterator it = mConnection.find(clientFD);
	if (it != mConnection.end())
	{
		// TODO: HTTP Response가 완성 되었는지 확인하는 함수가 필요함, Http 객체에 요청
		// TODO: 응답 코드가 5xx 일때 mConnection에서 currEvent->ident 삭제
		// TODO: mClient는 임시로 있는 자료구조 (HTTP Request를 받았는지 확인하기 위함, HTTP Response를 받았는지 확인해야 함)
		if (mClient[clientFD] != "")
		{
			std::string httpResponse = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nHello, World!"; // temp
			if (write(clientFD, httpResponse.c_str(), httpResponse.size()) == -1)
			{
				disconnectClient(clientFD);
				throw std::runtime_error("write() error");
			}
			else
				mClient[clientFD].clear();
		}
	}
}

void	WebServer::handleErrorEvent(int event)
{
	if (event == mListenFd)
		throw std::runtime_error("server socket error");
	else
	{
		disconnectClient(event);
		throw std::runtime_error("Client connection error");
	}
}

