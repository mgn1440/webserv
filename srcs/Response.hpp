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
    bool IsCGI() const;
    void MakeResponse(struct Request& req);
    void PrintResponse();
	void AppendCGIBody(const std::string& CGIBody);
    void SetStatusOf(int statusCode);
    void WriteResponseHeaderTo(int clientFD);
    void WriteResponseBodyTo(int clientFD);
    const char* GetABSPath() const;
    std::map<std::string, std::string> GetParams() const;
    void GenCGIBody();
    std::string GetCGIPath() const;
    void parseHeaderOfCGI();
    void SetRequestBody(const std::string& requestBody);
    std::string GetRequestBody();

    void TestMethod(); // debug
private:
    bool isValidMethod(struct Request& req, struct Resource& res);
    void processGET(struct Resource& res);
    void processPOST(struct Resource& res);
    // void processHEAD(struct Resource& res);
    // void processPUT(struct Resource& res);
    void processDELETE();
    void createResponseHeader();
    void createResponseBody(int statCode);

	void setFromResource(struct Resource);
    void setDate();

    bool mbCGI;
    bool mbAutoIndex;
    std::string mCGIPath;
    std::string mCGIExtension;
    std::string mStartLine;
    std::string mHeader;
    std::string mBody;
    size_t mBodySize;
    std::string mRequestBody;
	std::map<std::string, std::string> mParams;
    std::map<std::string, std::string> mHeaderMap;

    std::string mHttpVer;
    int mStatCode;
    std::string mStat;

    bool mbFile;
    bool mbDir;
    bool mbContentLen;
    std::string mABSPath;
	std::map<int, std::string> mErrorPage;
	bool mbConnectionStop;
};

#endif
