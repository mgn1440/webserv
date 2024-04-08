#include <iostream>
#include <vector>
#include <deque>
#include "ConfigHandler.hpp"
#include "parseUtils.hpp"
#include "Response.hpp"
#include "Request.hpp"

ConfigHandler* ConfigHandler::configHandler = NULL;

void setRequest(struct Request& request)
{
	request.statusCode = 200;
	request.parsedStatus = PARSED_NOT;
	request.startLine = "";
	request.method = "GET";
	request.URI = "/webserv/";
	request.domain = "www.ex1.com";
	request.params.clear();
	request.HTTPVersion = "HTTP/1.1";
	request.headers.clear();
	request.body = "";
	request.hostParsed = false;
	request.connectionStop = false;
}

int main(int ac, char** av)
{
    try
    {
        if (ac > 2)
            exitWithError("too many arguments");
        else if (ac == 1)
            ConfigHandler::MakeConfigHandler("./conf/default.conf");
        else if (ac == 2)
            ConfigHandler::MakeConfigHandler(std::string(av[1]));

        ConfigHandler& rhs = ConfigHandler::GetConfigHandler();

        struct Request req;
        setRequest(req);
        std::vector<struct Request> v;
        v.push_back(req);
        std::deque<Response> q =  rhs.GetResponseOf(v);
        q[0].PrintResponse();       
        // rhs.PrintAll();
    }
    catch(const std::exception& e)
    {
        exitWithError(e.what());
    }
    
}