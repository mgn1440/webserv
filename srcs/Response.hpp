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
    void SetStatusOf(int statusCode, std::string str);
	void WriteResponse(int clientFD);
    const char* GetABSPath() const;
    std::map<std::string, std::string> GetParams();
    void GenCGIBody();
    std::string GetCGIPath() const;
    void parseHeaderOfCGI();
    std::string& GetRequestBody();
	int GetSendStatus();
    void CreateResponseHeader();

    void TestMethod(); // debug
private:
    bool isValidMethod(struct Request& req, struct Resource& res);
    void processGET(struct Resource& res);
    void processPOST(struct Resource& res);
    // void processHEAD(struct Resource& res);
    // void processPUT(struct Resource& res);
    void processDELETE();
    void createResponseBody(int statCode);
	void respectiveSend(int clientFD, const std::string& toSend, int checkCond, int setCond);

	void setFromResource(struct Resource& res);
    void setDate();
	void setCGIParam(struct Request& req);

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

	size_t mRemainSendSize;
	int mSendStatus;	
	size_t mSendPos;
};

enum eSendFlags
{
	SEND_NOT = 0,
	SEND_START = 1,
	SEND_START_DONE = 1,
	SEND_HEADER = 2,
	SEND_HEADER_DONE = 3,
	SEND_BODY = 4,
	SEND_ALL = 7
};

#endif
