# pragma once 

#ifndef HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

# include <iostream>
# include <map>

# include "ConfigHandler"

class Server;

class HttpResponse
{
public :
	HttpResponse(std::string confFile = "./conf/default.conf");
	~HttpResponse();

private :
	HttpResponse(const HttpResponse& rhs);
	HttpResponse&	operator = (const HttpResponse& rhs);
	void			parseNginxConfig(std::string confFile);

	ConfigHandler	configHandler;
};

#endif