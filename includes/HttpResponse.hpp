#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include <iostream>
# include <map>
# include "ConfigHandler.hpp"

class Server;

class HttpResponse
{
public :
	HttpResponse(const ConfigHandler& configHandler);
	~HttpResponse();
private :
	HttpResponse(const HttpResponse& rhs);
	HttpResponse& operator = (const HttpResponse& rhs);
	const ConfigHandler& mConfigHandler;
};

#endif
