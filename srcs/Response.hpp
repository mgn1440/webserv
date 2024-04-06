#pragma once

#ifndef  RESPONSE_HPP
# define RESPONSE_HPP

# include <string>

class Response
{
public:
    Response();
    Response(const Response& rhs);
    Response& operator=(const Response& rhs);
    ~Response();
    bool IsCGI();
    std::string GetErrorPage(int errorCode);
    std::string GetResponse();
private:
    bool mCGI;
    std::string body;

};

#endif