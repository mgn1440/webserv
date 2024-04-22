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
# include "HttpHandler.hpp"

# define KQ_EVENT_SIZE 10000

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
		std::map<int, HttpHandler> mRequestMap; // key: clientFD, value: HttpHandler
		std::map<int, std::deque<Response> > mResponseMap; // key: clientFD, value: Response
		std::vector<int> mServSockList;
		std::vector<struct kevent> mChangeList;
		std::vector<std::string> mEnvList;
		std::map<int, std::pair<Response*, int> > mCGIPipeMap; // key: pipeFD, value: Response, clientFD
		std::map<int, std::pair<int,pid_t> > mCGIClientMap; // key: clientFD, value: pipeFD, PID
		std::map<pid_t, std::pair<Response*,int> > mCGIPidMap; // key: pid, value: Response, pipeFD
		std::map<int, std::pair<Response*, size_t> > mCGIPostPipeMap; // key: pipe(CGI STDIN_FILENO), value: Response, 이미 write 된 문자열 길이
		std::map<int, bool> mTimerMap; // key: clientFD, value: TimerOn Off;
		struct kevent mEventList[KQ_EVENT_SIZE];

		WebServ();
		WebServ& operator=(const WebServ&);
		WebServ(const WebServ&);
		void createServerSocket(const std::set<int>& portList);
		void setKqueue(void);
		void addEvents(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
		void runKqueueLoop(void);
		void acceptNewClientSocket(struct kevent* currEvent);
		void processHttpRequest(struct kevent* currEvent);
		void processCGI(Response& response, int clinetFD);
		//void sendCGIResource(struct kevent* currEvent);
		void writeHttpResponse(struct kevent* currEvent);
		void writeToCGIPipe(struct kevent* currEvent);
		void waitCGIProc(struct kevent* currEvent);
		void handleTimeOut(struct kevent* currEvent);
		bool isFatalKeventError(void);
		char *const *makeCGIEnvList(Response& response);
		char *const *makeArgvList(const std::string& CGIPath, const std::string& ABSPath);
		void sendPipeData(struct kevent* currEvent);
		void eraseCGIMaps(int pid, int clientFD, int pipeFD);
		void eraseClientMaps(int clientFD);
};

#endif
