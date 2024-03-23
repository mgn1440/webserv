#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP
# include <string>

class WebServer
{
private:
	WebServer(const WebServer& src);
	WebServer& operator=(const WebServer& rhs);
public:
	WebServer();
	WebServer(std::string conf_file = "default_path");
	~WebServer();
};

#endif