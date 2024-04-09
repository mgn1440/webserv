#pragma once

#ifndef  RESPONSE_HPP
# define RESPONSE_HPP

# include <string>
# include <cstdio>
# include "Request.hpp"

class ConfigHandler;

class Response
{
public:
    Response();
    Response(const Response& rhs);
    Response& operator=(const Response& rhs);
    ~Response();
    bool IsCGI();
    std::string GetResponse(struct Request& req);

    void PrintResponse();
    void CreateResponseHeader();
    void CreateResponseBody();
	void SetCGIBody(const std::string& CGIBody);
    void SetStatusOf(int statusCode);
private:

    bool isValidMethod(struct Request& req, struct Resource& res);
    void processGET(struct Resource& res);
    // void processPOST(struct Resource& res);
    // void processHEAD(struct Resource& res);
    // void processPUT(struct Resource& res);
    // void processDELETE(struct Resource& res);
	std::string genResponseMsg();
	void setFromResource(struct Resource);

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
	std::map<int, std::string> mErrorPage;
};

#endif