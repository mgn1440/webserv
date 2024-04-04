#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <map>
# include <deque>
# include <vector>
# include <string>
# include <sys/event.h>
# include "Request.hpp"
# include "Response.hpp"

// system call error 인해 webserv 프로그램이 종료되는 건 말이 안된다
// runKqueue 내부에서 throw catch 하는 구조로 만들어야 함
// TODO: clientFD read에서 Error랑, pipeFD read에서 Error는 어떻게 분기 처리를 해줘야 하는가?
class WebServ
{
	public:
		WebServ(std::vector<int> portList);
	private:
		int mKq;
		std::map<int, Request> mRequestMap;
		std::map<int, std::deque<Response> > mResponseMap;
		std::vector<int> mServSockList;
		std::vector<struct kevent> mChangeList;
		std::map<int, Response*> mCGIBodyBuf; // key: pipeFD, value: Response body(pipeFD에 읽기 요청이 들어오면 담당하는 Respons Buff에 저장한다.)
		struct kevent mEventList[30];

		WebServ();
		~WebServ();
		WebServ& operator=(const WebServ&);
		WebServ(const WebServ&);
		void createServerSocket(std::vector<int> portList);
		void setKqueue(void);
		void addEvents(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
		void runKqueueLoop(void);
		void acceptNewClientSocket(struct kevent* currEvent);
		void processHttpRequest(struct kevent* currEvent);
		void sendCGIResource(struct kevent* currEvent);
		std::string readHttpRequest(int clientFD);
};

#endif
