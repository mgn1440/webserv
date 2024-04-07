#pragma once

#ifndef  RESPONSE_HPP
# define RESPONSE_HPP

# include <string>

# include "Request.hpp"

class ConfigHandler;

class Response
{
public:
    Response(struct Request& rhs);
    Response(const Response& rhs);
    Response& operator=(const Response& rhs);
    ~Response();
    bool IsCGI();
    std::string GetErrorPage(int errorCode);
    std::string GetResponse();
private:
    ConfigHandler& mConfigHandler;
    bool mCGI;
    std::string body;

};

#endif