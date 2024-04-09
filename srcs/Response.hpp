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
    std::string GetResponse();

    void PrintResponse();
    void CreateResponseHeader();
    void CreateResponseBody();
    void SetStatusOf(int statusCode);
private:

    bool isValidMethod(struct Request& req, struct Resource& res);
    void processGET(struct Resource& res);
    void processPOST(struct Resource& res);
    void processHEAD(struct Resource& res);
    void processPUT(struct Resource& res);
    void processDELETE(struct Resource& res);

    bool mbCGI;
    bool mbAutoIndex;
    std::string mCGIPath;
    std::string mCGIExtension;
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