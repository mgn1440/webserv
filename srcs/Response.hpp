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
	ssize_t WriteResponse(int clientFD);
    const char* GetABSPath() const;
    std::map<std::string, std::string> GetParams();
    void GenCGIBody();
    std::string GetCGIPath() const;
    void parseHeaderOfCGI();
    std::string& GetRequestBody();
    bool IsConnectionStop() const;
	int GetSendStatus();
    void CreateResponseHeader();
    void TestMethod(); // debug
    void SetCGIInfo(int clientFd, int pipeRdFd, int pipeWrFd, int pid);
    pid_t GetPid();
    int GetClientFd();
    int GetWritePipeFd();
    int GetReadPipeFd();
    
    size_t Written;
private:
    bool isValidMethod(struct Request& req, struct Resource& res);
    void processGET(struct Resource& res, struct Request& req);
    void processPOST(struct Resource& res, struct Request& req);
    void processDELETE();
    void createResponseBody(int statCode);
	ssize_t respectiveSend(int clientFD, const std::string& toSend, int checkCond, int setCond);
	void setFromResource(struct Resource& res);
	void setFromRequest(struct Request& req);
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
    std::string mMethod;
	std::map<std::string, std::string> mParams;
    std::map<std::string, std::string> mHeaderMap;

    int mClientFd;
    int mPipeFd[2];
    pid_t mPid;
    std::string mHttpVer;
    int mStatCode;
    std::string mStat;

    bool mbFile;
    bool mbDir;
    bool mbContentLen;
    std::string mABSPath;
	std::map<int, std::string> mErrorPage;
	bool mbConnectionStop;

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
