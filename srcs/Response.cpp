#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <cstdio>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "ConfigHandler.hpp"
#include "convertUtils.hpp"
#include "Response.hpp"
#include "Resource.hpp"
#include "StatusPage.hpp"
#include "parseUtils.hpp"
#include "STLUtils.hpp"

std::string getIndexListOf(const std::string& URI, const std::string& absPath);

Response::~Response()
{}

Response::Response(const Response& rhs)
{
    mbCGI = rhs.mbCGI;
    mbAutoIndex = rhs.mbAutoIndex;
    mCGIPath = rhs.mCGIPath;
    mCGIExtension = rhs.mCGIExtension;
	mStartLine = rhs.mStartLine;
    mHeader = rhs.mHeader;
    mBody = rhs.mBody;
	mBodySize = rhs.mBodySize;
    mParams = rhs.mParams;
    mHttpVer = rhs.mHttpVer;
    mStatCode = rhs.mStatCode;
    mStat = rhs.mStat;
    mbFile = rhs.mbFile;
    mbDir = rhs.mbDir;
    mABSPath = rhs.mABSPath;
	mHeaderMap = rhs.mHeaderMap;
    mbContentLen = rhs.mbContentLen;
    mbConnectionStop = rhs.mbConnectionStop;
	mRequestBody = rhs.mRequestBody;
	mSendStatus = rhs.mSendStatus;
	mSendPos = rhs.mSendPos;
	mClientFd = rhs.mClientFd;
	mPipeFd[0] = rhs.mPipeFd[0];
	mPipeFd[1] = rhs.mPipeFd[1];
	mPid = rhs.mPid;
	Written = rhs.Written;
}

Response& Response::operator=(const Response& rhs)
{
    if (&rhs == this)
		return *this;
	mbCGI = rhs.mbCGI;
    mbAutoIndex = rhs.mbAutoIndex;
    mCGIPath = rhs.mCGIPath;
    mCGIExtension = rhs.mCGIExtension;
	mStartLine = rhs.mStartLine;
    mHeader = rhs.mHeader;
    mBody = rhs.mBody;
	mBodySize = rhs.mBodySize;
    mParams = rhs.mParams;
    mParams = rhs.mParams;
    mHttpVer = rhs.mHttpVer;
    mStatCode = rhs.mStatCode;
    mStat = rhs.mStat;
    mbFile = rhs.mbFile;
    mbDir = rhs.mbDir;
    mABSPath = rhs.mABSPath;
	mHeaderMap = rhs.mHeaderMap;
    mbContentLen = rhs.mbContentLen;
    mbConnectionStop = rhs.mbConnectionStop;
	mRequestBody = rhs.mRequestBody;
	mSendStatus = rhs.mSendStatus;
	mSendPos = rhs.mSendPos;
	mClientFd = rhs.mClientFd;
	mPipeFd[0] = rhs.mPipeFd[0];
	mPipeFd[1] = rhs.mPipeFd[1];
	mPid = rhs.mPid;
	Written = rhs.Written;
	return *this;
}

Response::Response()
	: Written(0)
    , mbCGI()
    , mbAutoIndex()
    , mCGIPath()
    , mCGIExtension()
    , mStartLine()
    , mHeader()
    , mBody()
	, mBodySize(0)
    , mParams()
	, mClientFd(-1)
	, mPid(-1)
    , mHttpVer("HTTP/1.1")
    , mStatCode()
    , mStat()
    , mbFile()
    , mbDir()
    , mbContentLen()
    , mABSPath()
    , mbConnectionStop()
    , mSendStatus()
    , mSendPos(0)
{
	mPipeFd[0] = -1;
	mPipeFd[1] = -1;
	mHeaderMap["Server"] = "Webserv";
}

void Response::MakeResponse(struct Request& req)
{
    struct Resource res = ConfigHandler::GetConfigHandler().GetResource(req.Port, req.URI);
	if (res.RedirCode)
	{
		SetStatusOf(res.RedirCode, res.Location);
		return;
	}
	setFromRequest(req);
	setFromResource(res);
	setDate();
	setCGIParam(req);
    if (req.StatusCode || !isValidMethod(req, res))
 	{
		SetStatusOf(req.StatusCode, "");
		return;
	}
	if (req.Method == "GET" || req.Method == "HEAD")
		processGET(res, req);
	else if (req.Method == "POST")
		processPOST(res, req);
	else if (req.Method == "DELETE")
		processDELETE();
}

void Response::PrintResponse()
{
	std::cout << "\033[1;32m" << "~~Print Response~~" << "\033[0m" << std::endl;
	std::string ret;
    ret = mStartLine;
	ret += mHeader;
	ret += mBody.substr(0, 50);
	std::cout << "\033[1;32m" << ret << "\033[0m" << "\n\n";
}

bool Response::isValidMethod(struct Request& req, struct Resource& res)
{
    if (find(res.HttpMethod.begin(), res.HttpMethod.end(), req.Method) == res.HttpMethod.end())
    {
        req.StatusCode = 405;
		return (false);
	}
	return (true);
}

void Response::processGET(struct Resource& res, struct Request& req)
{
	struct stat statBuf;
	if (stat(res.ABSPath.c_str(), &statBuf) == -1)
	{
		SetStatusOf(404, "");
		return;
	}
	if (S_ISDIR(statBuf.st_mode))
	{
		mbDir = true;
		std::set<std::string>::iterator indexName = res.Index.begin();
		for (; indexName != res.Index.end(); ++indexName)
		{
			if (stat((mABSPath + "/" + (*indexName)).c_str(), &statBuf) == 0)
			{
				mABSPath += ("/" + *indexName);
				mbDir = false;
				mbFile = true;
				break;
			}
		}
		if (!mbFile)
		{
			if (mbAutoIndex)
			{
				mBody = getIndexListOf(res.URI, mABSPath);
				mHeaderMap["Content-Type"] = "text/html";
			}
			else if (!mbAutoIndex)
			{
				SetStatusOf(404, "");
				return;
			}
		}
	}
	else
		mbFile = true;
	if (mbFile)
	{
		mHeaderMap["Content-Type"] = ConfigHandler::GetConfigHandler().GetContentType(mABSPath);
		mCGIExtension = mABSPath.substr(mABSPath.find_last_of(".") + 1);
		if (res.CGIBinaryPath.find(mCGIExtension) != res.CGIBinaryPath.end())
		{
			mbCGI = true;
			mCGIPath = res.CGIBinaryPath[mCGIExtension];
			setCGIParam(req);
		}
		else
			mCGIPath = "";
	}
	if (!mbCGI)
	{
		std::ifstream ifs(mABSPath);
		createResponseBody(200);
	}
	else
		mStatCode = 200;
}

void Response::processPOST(struct Resource& res, struct Request& req)
{
	struct stat statBuf;
	if (stat(mABSPath.c_str(), &statBuf) == -1)
	{
		std::ofstream ofs(mABSPath, std::ios::out);
		if (ofs.fail())
			exitWithError("File Error");
		ofs << mRequestBody;
		stat(mABSPath.c_str(), &statBuf);
	}
	if (S_ISDIR(statBuf.st_mode))
		mbDir = true;
	else
        mbFile = true;
	if (mbFile)
	{
		mHeaderMap["Content-Type"] = ConfigHandler::GetConfigHandler().GetContentType(mABSPath);
		mCGIExtension = mABSPath.substr(mABSPath.find_last_of(".") + 1);
		if (res.CGIBinaryPath.find(mCGIExtension) != res.CGIBinaryPath.end())
		{
			mbCGI = true;
			mCGIPath = res.CGIBinaryPath[mCGIExtension];
			setCGIParam(req);
		}
		else
			mCGIPath = "";
	}
	if (!mbCGI){
		SetStatusOf(204, "");
		return;
	}
}

void Response::processDELETE()
{
	struct stat statBuf;
	if (stat(mABSPath.c_str(), &statBuf) == -1 || S_ISDIR(statBuf.st_mode))
	{
		SetStatusOf(204, "");
		return;
	}
    mbFile = true;
    if (std::remove(mABSPath.c_str()))
	{
		mStatCode = 403;
		mBody = "";
		return;
	}
    mHeaderMap["Content-Type"] = "application/json";
	mBody = "{\n \"message\": \"Item deleted successfully.\"\n}";
}

void Response::SetStatusOf(int statusCode, std::string str)
{
    mStatCode = statusCode;
	if (mbConnectionStop)
		mHeaderMap["Connection"] = "close";
	if (mStatCode == 204)
	{
		mHeaderMap["Content-Type"] = "text/html";
		mBody = "";
	}
	else if (mStatCode / 100 == 3)
	{
		mHeaderMap["Location"] = str;
		mBody = "";
	}
	else if (mErrorPage.find(statusCode) != mErrorPage.end())
	{
		mABSPath = mErrorPage[statusCode];
		createResponseBody(statusCode);
	}
	else
	{
		mStatCode = statusCode;
		mHeaderMap["Content-Type"] = "text/html";
		mBody = StatusPage::GetInstance()->GetStatusPageOf(statusCode);
	}
}

std::string& Response::GetRequestBody()
{
	return (mRequestBody);
}


bool Response::IsCGI() const
{
	return mbCGI;
}

void Response::AppendCGIBody(const std::string& CGIBody)
{
	mBody += CGIBody;
}

void Response::GenCGIBody()
{
	mHeaderMap["Content-Type"] = "text/html";
	parseHeaderOfCGI();
}


std::string Response::GetCGIPath() const
{
	return (mCGIPath);
}


ssize_t Response::WriteResponse(int clientFD)
{
	ssize_t writeSize = 0;
	writeSize += respectiveSend(clientFD, mStartLine, SEND_NOT, SEND_START);
	writeSize += respectiveSend(clientFD, mHeader, SEND_START_DONE, SEND_HEADER);
	writeSize += respectiveSend(clientFD, mBody, SEND_HEADER_DONE, SEND_BODY);
	return (writeSize);
}

void Response::SetCGIInfo(int clientFd, int pipeRdFd, int pipeWrFd, int pid)
{
	mClientFd = clientFd;
	mPipeFd[0] = pipeRdFd;
	mPipeFd[1] = pipeWrFd;
	mPid = pid;
}

pid_t Response::GetPid()
{
	return (mPid);
}

int Response::GetClientFd()
{
	return (mClientFd);
}

int Response::GetReadPipeFd()
{
	return (mPipeFd[0]);
}

int Response::GetWritePipeFd()
{
	return (mPipeFd[1]);
}

ssize_t Response::respectiveSend(int clientFD, const std::string& toSend, int checkCond, int setCond)
{
	if (checkCond == SEND_HEADER_DONE && mMethod == "HEAD")
		return (0);
	if (mSendStatus == checkCond)
	{
		ssize_t writeSize = write(clientFD, toSend.c_str() + mSendPos, toSend.size() - mSendPos);
		if (writeSize == -1)
			throw std::runtime_error("write error: write response");
		mSendPos += writeSize;
		if (mSendPos == toSend.size())
		{
			mSendStatus |= setCond;
			mSendPos = 0;
		}
		return (writeSize);
	}
	return (0);
}

void Response::CreateResponseHeader()
{
    mStartLine = mHttpVer + " ";
    mStartLine += intToString(mStatCode) + " " + StatusPage::GetInstance()->GetStatusMessageOf(mStatCode) +"\r\n";
	mHeader = "";
	for (std::map<std::string, std::string>::iterator it = mHeaderMap.begin(); it != mHeaderMap.end(); it ++)
		mHeader += it->first + ": " + it->second + "\r\n";
    mHeader += "Content-length: " + intToString(mBody.size()) + "\r\n";
	mHeader += "\r\n";
}

void Response::createResponseBody(int statCode)
{
	std::ifstream ifs(mABSPath);
    if (ifs.fail())
        throw std::runtime_error("file open error");
	if (statCode != 0)
		mStatCode = statCode;
	else
		mStatCode = 200;
    char buf[65535];
    do
    {
		ifs.read(buf, sizeof(buf));
        mBody += std::string(buf, ifs.gcount());
    } while (ifs.gcount());
}


void Response::setFromResource(struct Resource& res)
{
	mbAutoIndex = res.BAutoIndex;
	mErrorPage = res.ErrorPage;
	mABSPath = res.ABSPath;
}

void Response::setFromRequest(struct Request& req)
{
	mbConnectionStop = req.ConnectionStop;
	mRequestBody = req.Body;
	mMethod = req.Method;
}

void Response::setDate()
{
    char buf[100];
    std::time_t time = std::time(NULL);
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", std::localtime(&time));
	mHeaderMap["Date"] = std::string(buf);
}

void Response::parseHeaderOfCGI()
{
	bool isHeader = true;


	std::istringstream ss(mBody);
	std::string line;
	while (std::getline(ss, line))
	{
		if (ss.eof())
			return;
		size_t idx = line.find(": ");
		if (line == "\r")
			break;
		if (line == "")
			continue;
		else if (line.back() != '\r' || idx == std::string::npos)
		{
			isHeader = false;
			break;
		}
		line.erase(line.size() - 1);
		std::string key = line.substr(0, idx);
		std::string val = line.substr(idx + 2);
		if (key == "Status")
		{
			size_t idx = val.find(' ');
			if (idx == std::string::npos)
			{
				isHeader = false;
				break;
			}
			mStatCode = std::atoi(val.substr(0, idx).c_str());
			mStat = val.substr(idx + 1);
		}
		else
			mHeaderMap[key] = val;
	}
	if (isHeader)
	{
		size_t idx = mBody.find("\r\n\r\n");
		if (idx != std::string::npos)
			mBody = mBody.substr(idx + 4);
	}
}

const char* Response::GetABSPath() const
{
	return (mABSPath.c_str());
}

std::map<std::string, std::string> Response::GetParams()
{
	return (mParams);
}


bool startWith(const std::string& str, std::string comp, char del)
{
	if (str.size() < comp.size())
		return false;
	return str.substr(0, str.find_first_of(del)) == comp;
}

void Response::setCGIParam(struct Request& req)
{
	// mParams["AUTH_TYPE"] = // authentication type
	mParams["CONTENT_LENGTH"] = intToString(req.Body.size());
	mParams["CONTENT_TYPE"] = req.Headers["Content-Type"];
	mParams["GATEWAY_INTERFACE"] = "CGI/1.1";
	mParams["PATH_INFO"] = req.URI; // after scirpt file, not full path
	// mParams["PATH_TRANSLATED"] = mABSPath; // root path + PATH_INFO, nowdays not use
	mParams["QUERY_STRING"] = req.query;
	// mParams["REMOTE_ADDR"] = // client 의 ip 주소, getaddrinfo 사용
	// mParams["REMOTE_HOST"] =  // domain name
	// mParams["REMOTE_IDENT"] = // ident protocal
	// mParams["REMOTE_USER"] = // if use auth then user's id
	mParams["REQUEST_METHOD"] = req.Method;
	// mParams["SCRIPT_NAME"] =  //before scirpt file
	mParams["SERVER_NAME"] = req.Domain;
	mParams["SERVER_PORT"] = intToString(req.Port);
	mParams["SERVER_PROTOCOL"] = "HTTP/1.1";
	mParams["SERVER_SOFTWARE"] = "webserv";
	for (std::map<std::string, std::string>::iterator it = req.Headers.begin(); it != req.Headers.end(); it++)
		if (startWith(it->first, "X", '-'))
			mParams["HTTP_" + it->first] = it->second;
}

bool Response::IsConnectionStop() const
{
	return (mbConnectionStop);
}


int Response::GetSendStatus()
{
	return mSendStatus;
}
