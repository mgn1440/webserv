#ifndef WEB_SERVER_HPP
# define WEB_SERVER_HPP


# include <vector>
# include <map>
# include <string>
# include <sys/event.h>

class WebServer
{
	public:
		WebServer(const std::string& config = "./conf/default.conf");
		WebServer& operator=(const WebServer& rhs);
		void	SetTCP(void);
		void	RunTCP(void);
	private:
		std::string					mConfig;
		int							mPort;
		int							mListenFd;
		int							mKq;
		std::map<int, std::string>	mClient;
		std::vector<struct kevent>	mChangeList;
		struct kevent				mEventList[8];
		void	changeEvents(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
		void	disconnectClient(int clientSocket);
};

#endif
