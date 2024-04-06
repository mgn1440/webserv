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
// TODO: clientFD read에서 Error랑, pipeFD read에서 Error는 어떻게 분기 처리를 해줘야 하는가?ㅂ
class WebServ
{
	public:
		WebServ(std::vector<int> portList);
	private:
		int mKq;
		std::map<int, HttpRequest> mRequestMap;
		std::map<int, std::deque<Response> > mResponseMap;
		std::vector<int> mServSockList;
		std::vector<struct kevent> mChangeList;
		std::map<int, std::pair<Response*, int> > mPipeMap; // key: pipeFD, value: Response, clientFD)
		std::map<int, std::pair<int, int> > mClientFDMap; // key: clientFD, value: pipeFD, PID
		std::map<int, std::pair<Response*, int> > mPidMap; // key: pid, value: Response, pipeFD
		// client fd를 key로 가져가야 하는거 아닌가?
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
		void writeHttpResponse(struct kevent* currEvent);
		void waitCGIProc(struct kevent* currEvent);
		void handleTimeOut(struct kevent* currEvent);
		bool isFatalKeventError(void);
		std::string readFDData(int clientFD);
};

// 이벤트 삭제 및 자료구조 삭제해야 하는 순간.

// processHttpRequest()에서 EV_EOF가 되면, clientFD를 이벤트에서 삭제

#endif
