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

//  TODO: Static이라고 가정 후 Response init을 돌린다.
//        추후 cgi를 돌릴 때 실제 ContentType과 ContentLength, status code 등을 설정한다.
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
	mRequestBody = rhs.mRequestBody;
	mSendStatus = rhs.mSendStatus;
	mSendPos = rhs.mSendPos;
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
	mRequestBody = rhs.mRequestBody;
	mSendStatus = rhs.mSendStatus;
	mSendPos = rhs.mSendPos;
    return *this;
}

Response::Response()
    : mbCGI()
    , mbAutoIndex()
    , mCGIPath()
    , mCGIExtension()
    , mStartLine()
    , mHeader()
    , mBody()
	, mBodySize(0)
    , mParams()
    , mHttpVer("HTTP/1.1")
    , mStatCode()
    , mStat()
    , mbFile()
    , mbDir()
    , mbContentLen()
    , mABSPath()
    , mSendStatus()
    , mSendPos(0)
{
	mHeaderMap["Server"] = "Webserv";
}

void Response::MakeResponse(struct Request& req)
{
    struct Resource res = ConfigHandler::GetConfigHandler().GetResource(req.Port, req.URI);
    mParams = req.Params;
	mbConnectionStop = req.ConnectionStop;
	mRequestBody = req.Body;
	if (res.RedirCode)
	{
		SetStatusOf(res.RedirCode, res.Location);
		return;
	}
	setFromRequest(req);
	setFromResource(res);
	setDate();
	setCGIParam(req);
    if (req.StatusCode || !isValidMethod(req, res)) // TODO: http ver, method, abs path
 	{
		SetStatusOf(req.StatusCode, "");
        return ;
	}
	if (req.Method == "GET" || req.Method == "HEAD")
		processGET(res);
	else if (req.Method == "POST")
		processPOST(res);
	else if (req.Method == "DELETE")
		processDELETE();
}

void Response::PrintResponse()
{
	std::cout << "\033[1;32m" << "~~Print Response~~" << "\033[0m" << std::endl;
	std::cout << "\033[1;32m" << "ABSPath: " <<  mABSPath << "\033[0m" << std::endl;  // debug
	std::string ret;
    ret = mStartLine;
	ret += mHeader;
	ret += mBody.substr(0, 50);
	std::cout << "\033[1;32m" << ret << "\033[0m" << "\n\n";
	// [HTTP version] [stat code] [status]
}

bool Response::isValidMethod(struct Request& req, struct Resource& res)
{
    if (find(res.HttpMethod.begin(), res.HttpMethod.end(), req.Method) == res.HttpMethod.end())
    {
        req.StatusCode = 405; // 501
        return (false);
    }
    return (true);
}

// TODO:
// URL의 마지막 파일 or 디렉터리인지 확인
// if (File) => mbFile = true, mbDir = fasle;
// if (Dir)
// std::set<std::string> index를 순회하면서 파일이 존재하는지 확인 (URL + index)
//
void Response::processGET(struct Resource& res)
{
	struct stat statBuf;
	if (stat(res.ABSPath.c_str(), &statBuf) == -1)
	{
		SetStatusOf(404, "");
		return ;
	}
	if (S_ISDIR(statBuf.st_mode))
	{
		mbDir = true;
		std::set<std::string>::iterator indexName = res.Index.begin();
		for (; indexName != res.Index.end(); ++indexName)
		{
			if (stat((mABSPath + "/" + (*indexName)).c_str(), &statBuf) == 0){
				// std::cout << mABSPath << ", " << (*indexName) << "\n" << std::endl; // debug
				mABSPath += ("/" + *indexName);
				mbDir = false;
				mbFile = true;
				break;
			}
		}
		if (!mbFile){
			if (mbAutoIndex){
				mBody = getIndexListOf(res.URI, mABSPath);
				mHeaderMap["Content-Type"] = "text/html";
			}
			else if (!mbAutoIndex)
			{
				SetStatusOf(404, "");
				return ;
			}
		}
	}
	else
		mbFile = true;
	if (mbFile){
		mHeaderMap["Content-Type"] = ConfigHandler::GetConfigHandler().GetContentType(mABSPath);
		mCGIExtension = mABSPath.substr(mABSPath.find_last_of(".") + 1);
		if (res.CGIBinaryPath.find(mCGIExtension) != res.CGIBinaryPath.end()) // is CGI
		{
			mbCGI = true;
			mCGIPath = res.CGIBinaryPath[mCGIExtension];
			// setCGIParam(req); request 여기까지 끌고 와야함
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

void Response::processPOST(struct Resource& res)
{
	struct stat statBuf;
	if (stat(mABSPath.c_str(), &statBuf) == -1)
	{
		// TODO: save Request Body to division
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
	if (mbFile){
		mHeaderMap["Content-Type"] = ConfigHandler::GetConfigHandler().GetContentType(mABSPath);
		mCGIExtension = mABSPath.substr(mABSPath.find_last_of(".") + 1);
		if (res.CGIBinaryPath.find(mCGIExtension) != res.CGIBinaryPath.end()) // is CGI
		{
			mbCGI = true;
			mCGIPath = res.CGIBinaryPath[mCGIExtension];
			// setCGIParam(req); request 여기까지 끌고 와야함
		}
		else
			mCGIPath = "";
	}
	if (!mbCGI){
		SetStatusOf(204, "");
		return ;
    }
}

void Response::processDELETE()
{
	struct stat statBuf;
	if (stat(mABSPath.c_str(), &statBuf) == -1 || S_ISDIR(statBuf.st_mode))
	{
		SetStatusOf(204, "");
		return ;
	}
    mbFile = true;
    if (std::remove(mABSPath.c_str()))
	{
		mStatCode = 403;
		mBody = "";
		return ;
	}
    mHeaderMap["Content-Type"] = "application/json";
	mBody = "{\n \"message\": \"Item deleted successfully.\"\n}";
}

void Response::SetStatusOf(int statusCode, std::string str)
{
    mStatCode = statusCode;
	if (mStatCode == 204)
	{
		mHeaderMap["Content-Type"] = "text/html";
		mBody = "";
	}
	if (mStatCode / 100 == 3)
	{
		mHeaderMap["Location"] = str;
		mBody = "";
	}
	else if (mErrorPage.find(statusCode) != mErrorPage.end()){
		mABSPath = mErrorPage[statusCode];
		createResponseBody(statusCode);
		// std::cerr << "Set status: " << statusCode << std::endl;
		// createResponseHeader();
	}
	else{
		// TODO: 204 같은 NO Content도 body를 만들어 주는가?
		mStatCode = statusCode;
		mHeaderMap["Content-Type"] = "text/html";
		mBody = StatusPage::GetInstance()->GetStatusPageOf(statusCode);
		// createResponseHeader();
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
	// std::cout << CGIBody.size() << std::endl; // debug
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

ssize_t Response::respectiveSend(int clientFD, const std::string& toSend, int checkCond, int setCond)
{
	if (checkCond == SEND_HEADER_DONE && mMethod == "HEAD")
		return (0);
	if (mSendStatus == checkCond){
		ssize_t writeSize = write(clientFD, toSend.c_str() + mSendPos, toSend.size() - mSendPos);
		if (writeSize == -1)
			throw std::runtime_error("write error: write response");
		mSendPos += writeSize;
		if (mSendPos == toSend.size()){
			mSendStatus |= setCond;
			mSendPos = 0;
		}
		return (writeSize);
	}
	return (0);
}

void Response::CreateResponseHeader()
{
	// body should complete when call this function
    mStartLine = mHttpVer + " ";
    mStartLine += intToString(mStatCode) + " " + StatusPage::GetInstance()->GetStatusMessageOf(mStatCode) +"\r\n";

	mHeader = "";
	for (std::map<std::string, std::string>::iterator it = mHeaderMap.begin(); it != mHeaderMap.end(); it ++)
	{
		mHeader += it->first + ": " + it->second + "\r\n";
	}
    // if (mHeaderMap.find("Content-Type") == mHeaderMap.end())
    //     mHeader += "Content-Type: type/plain\r\n";
    mHeader += "Content-length: " + intToString(mBody.size()) + "\r\n";
	mHeader += "\r\n";
}

// TODO: directory가 들어왔을 때 autoindex on =>  directory listing page
// directory가 들어왔을 때 autoindex off => index file name이 붙어야 함
void Response::createResponseBody(int statCode)
{
	// std::cerr << "mStatus: " << mStatCode << ", Status: " << statCode << std::endl;
	std::ifstream ifs(mABSPath);
    if (ifs.fail())
        throw std::runtime_error("file open error");
	if (statCode != 0)
		mStatCode = statCode;
	else
		mStatCode = 200;
	// std::cerr << "mStatus: " << mStatCode << ", Status: " << statCode << std::endl;
    char buf[65535];
    do
    {
		ifs.read(buf, sizeof(buf));
		// mBodySize += ifs.gcount();
		// buf[ifs.gcount()] = '\0';
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
    mParams = req.Params;
	mbConnectionStop = req.ConnectionStop;
	mRequestBody = req.Body;
	mMethod = req.Method;
}

void Response::setDate()
{
    char buf[100];
    std::time_t time = std::time(NULL);
	// Date: [Day], [date] [Month] [year] [time] KST;
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", std::localtime(&time));
	mHeaderMap["Date"] = std::string(buf);
}

void Response::parseHeaderOfCGI()
{
	bool isHeader = true;

	// debug
	// mBody = "Content-Type: text/html\r\nX-Powered-By: Python/CGI\r\nStatus: 200 OK\r\n\r\nHello World!";
	// debug

	std::istringstream ss(mBody);
	std::string line;
	// std::cout << mBody << "\n\n";
	while (std::getline(ss, line))
	{
		// std::cout << "line: " << line << std::endl;
		if (ss.eof())
			return;
		size_t idx = line.find(": ");
		if (line == "\r")
			break;
		if (line == "") // temp
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
		// std::cout << "key: " << key << ", val: " << val << std::endl;
	}
	if (isHeader)
	{
		size_t idx = mBody.find("\r\n\r\n");
		if (idx != std::string::npos)
			mBody = mBody.substr(idx + 4);
	}
	// std::cerr << "body\n" << mBody;
}

const char* Response::GetABSPath() const
{
	return (mABSPath.c_str());
}

std::map<std::string, std::string> Response::GetParams()
{
	// std::cout << "mParams" << std::endl; // debug
	// printMap(mParams); // debug
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
	if (req.URI.find('?') != std::string::npos)
		mParams["QUERY_STRING"] = req.URI.substr(req.URI.find('?'));
	// mParams["REMOTE_ADDR"] = // client 의 ip 주소, getaddrinfo 사용
	// mParams["REMOTE_HOST"] =  // domain name
	// mParams["REMOTE_IDENT"] = // ident protocal
	// mParams["REMOTE_USER"] = // if use auth then user's id
	mParams["REQUEST_METHOD"] = req.Method;
	// mParams["SCRIPT_NAME"] =  //before scirpt file
	mParams["SERVER_NAME"] = req.Domain;
	mParams["SERVER_PORT"] = req.Port;
	mParams["SERVER_PROTOCOL"] = "HTTP/1.1";
	mParams["SERVER_SOFTWARE"] = "webserv";
	for (std::map<std::string, std::string>::iterator it = req.Headers.begin(); it != req.Headers.end(); it++){
		if (startWith(it->first, "X", '-')){
			mParams["HTTP_" + it->first] = it->second;
		}
	}
}

bool Response::IsConnectionStop() const
{
	return (mbConnectionStop);
}

void Response::TestMethod()
{
	std::cout << "Request Body size: " << mRequestBody.size() << std::endl;
}

int Response::GetSendStatus()
{
	return mSendStatus;
}
