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
# define TIMEOUT_SIZE 10000

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
		std::map<int, Response*> mCGIPipeMap; // key: readPipeFD, value: Response pointer
		std::map<int, Response*> mCGIClientMap;  // key: clientFD, value: Response pointer  
		std::map<int, Response*> mCGIPidMap; // key: pid, value: Response pointer
		std::map<int, Response*> mCGIPostPipeMap; // key: writePipeFD, value: Response pointer
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
		void writeHttpResponse(struct kevent* currEvent);
		void writeToCGIPipe(struct kevent* currEvent);
		void waitCGIProc(struct kevent* currEvent);
		void handleTimeOut(struct kevent* currEvent);
		bool isFatalKeventError(void);
		char *const *makeCGIEnvList(Response& response);
		char *const *makeArgvList(const std::string& CGIPath, const std::string& ABSPath);
		void sendPipeData(struct kevent* currEvent);
		void eraseCGIMaps(Response* res);
		void eraseClientMaps(int clientFD);
};

#endif
