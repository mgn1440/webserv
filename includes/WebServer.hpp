#ifndef WEB_SERVER_HPP
# define WEB_SERVER_HPP

# include <vector>
# include <map>
# include <string>
# include <sys/event.h>
# include "Http.hpp"
# include "ConfigHandler.hpp"

class WebServer
{
	public:
		WebServer(const std::string& config);
		~WebServer();
		void SetTCP(void);
		void RunTCP(void);
	private:
		int mPort;
		int mListenFd;
		int mKq;
		ConfigHandler mConfigHandler;
		std::map<int, Http*> mConnection;
		std::map<int, std::string> mClient;
		std::vector<struct kevent> mChangeList;
		struct kevent mEventList[8];

		WebServer(const WebServer& rhs);
		WebServer& operator=(const WebServer& rhs);
		void changeEvents(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
		void disconnectClient(int clientSocket);
		void acceptNewClientSocket(void);
		void sendRequestToHttp(int clientFD);
		void writeResponseToClient(int clientFD);
		void handleErrorEvent(int event);
};

#endif

