#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <map>
# include <deque>
# include <vector>
# include <string>
# include <set>
# include <sys/event.h>
# include "Request.hpp"
# include "Response.hpp"
# include "HttpRequest.hpp"

// system call error 인해 webserv 프로그램이 종료되는 건 말이 안된다
// runKqueue 내부에서 throw catch 하는 구조로 만들어야 함
// TODO: clientFD read에서 Error랑, pipeFD read에서 Error는 어떻게 분기 처리를 해줘야 하는가?ㅂ
class WebServ
{
	public:
		WebServ(const std::set<int>& portList, const std::vector<std::string>& env);
		~WebServ();
	private:
		int mKq;
		std::map<int, int> mServSockPortMap; // key: servSocket, val: port
		std::map<int, HttpRequest> mRequestMap;
		std::map<int, std::deque<Response> > mResponseMap;
		std::vector<int> mServSockList;
		std::vector<struct kevent> mChangeList;
		std::vector<std::string> mEnvList;
		std::map<int, std::pair<Response&, int> > mCGIPipeMap; // key: pipeFD, value: Response body(pipeFD에 읽기 요청이 들어오면 담당하는 Respons Buff에 저장한다.)
		std::map<int, std::pair<int,int> > mCGIClientMap; // key: clientFD, value: pipeFD, PID
		std::map<int, std::pair<Response&,int> > mCGIPidMap; // key: pid, value: Response, pipeFD
		struct kevent mEventList[30];

		WebServ();
		WebServ& operator=(const WebServ&);
		WebServ(const WebServ&);
		void createServerSocket(const std::set<int>& portList);
		void setKqueue(void);
		void addEvents(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
		void runKqueueLoop(void);
		void acceptNewClientSocket(struct kevent* currEvent);
		void processHttpRequest(struct kevent* currEvent);
		void processCGI(const Response& response, int clinetFD);
		//void sendCGIResource(struct kevent* currEvent);
		void writeHttpResponse(struct kevent* currEvent);
		void waitCGIProc(struct kevent* currEvent);
		void handleTimeOut(struct kevent* currEvent);
		bool isFatalKeventError(void);
		std::string readFDData(int clientFD);
		const char **makeCGIEnvList(const Response& response)
		//void processGetCGI(const Request& request, const Response& response, int clientFD); // pipe 1개
		//void processPostCGI(const Request& request, const Response& response, int clientFD); // pipe 2개, 표준입력으로 Http Request Body로 줘야 함
};

#endif
