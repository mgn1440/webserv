#pragma once

#ifndef  RESPONSE_HPP
# define RESPONSE_HPP

# include <string>

# include "Request.hpp"

class ConfigHandler;

class Response
{
public:
    Response();
    Response(struct Request& request);
    Response(const Response& rhs);
    Response& operator=(const Response& rhs);
    ~Response();
    bool IsCGI();
    std::string GetErrorPage(int errorCode);
    std::string GetResponse();

    void CreateResponseHeader();
    void CreateResponseBody();
    void PrintResponse();
private:

    bool isValidStartLine(struct Request& req, struct Resource& res);

    bool mbCGI;
    bool mbAutoIndex;
    std::string mCGI;
    std::string mStartLine;
    std::string mHeader;
    std::string mBody;

    std::string mHttpVer;
    int mStatCode;
    std::string mStat;

    bool mbFile;
    bool mbDir;
    std::string mABSPath;
    std::string mDate;
    std::string mServer;
    bool mbContentLen;
    std::string mContentType;
    
};

#endif