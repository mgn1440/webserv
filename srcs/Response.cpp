#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
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
#include "STLUtils.hpp"

//  TODO: Static이라고 가정 후 Response init을 돌린다.
//        추후 cgi를 돌릴 때 실제 ContentType과 ContentLength, status code 등을 설정한다.
std::string getIndexListOf(const std::string& path);

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
    mParams = rhs.mParams;
    mHttpVer = rhs.mHttpVer;
    mStatCode = rhs.mStatCode;
    mStat = rhs.mStat;
    mbFile = rhs.mbFile;
    mbDir = rhs.mbDir;
    mABSPath = rhs.mABSPath;
	mHeaderMap = rhs.mHeaderMap;
    mbContentLen = rhs.mbContentLen;
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
    mParams = rhs.mParams;
    mHttpVer = rhs.mHttpVer;
    mStatCode = rhs.mStatCode;
    mStat = rhs.mStat;
    mbFile = rhs.mbFile;
    mbDir = rhs.mbDir;
    mABSPath = rhs.mABSPath;
	mHeaderMap = rhs.mHeaderMap;
    mbContentLen = rhs.mbContentLen;
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
    , mParams()
    , mHttpVer("HTTP/1.1")
    , mStatCode()
    , mStat()
    , mbFile()
    , mbDir()
    , mbContentLen()
    , mABSPath()
{
	mHeaderMap["Server"] = "Webserv";
}

void Response::createResponseHeader()
{
	// body should complete when call this function
    mStartLine = mHttpVer + " ";
    mStartLine += intToString(mStatCode) + " " + StatusPage::GetInstance()->GetStatusMessageOf(mStatCode) +"\r\n";
	
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
void Response::createResponseBody()
{
	std::ifstream ifs(mABSPath);
    if (ifs.fail())
        throw std::runtime_error("file open error");
	mStatCode = 200;
    char buf[16384];
    do
    {
		ifs.read(buf, sizeof(buf) - 1);
		buf[ifs.gcount()] = '\0';
        mBody += std::string(buf);
    } while (ifs.gcount());
}


bool Response::isValidMethod(struct Request& req, struct Resource& res)
{
    if (find(res.HttpMethod.begin(), res.HttpMethod.end(), req.method) == res.HttpMethod.end())
    {
        req.statusCode = 501;
        return (false);
    }
    return (true);
}

// Debug

void Response::PrintResponse()
{
	std::string ret;
    std::string temp;

	// [HTTP version] [stat code] [status]
	ret += mHttpVer + " ";
    std::stringstream ss;
    ss << mStatCode;
    ss >>  temp;
    ret += temp + " " + "OK" +"\r\n";
    createResponseBody();
    std::cout << ret << std::endl;
	printMap(mHeaderMap);
    std::cout << mBody << std::endl;
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
	if (stat(mABSPath.c_str(), &statBuf) == -1)
	{
		SetStatusOf(404);
		return ;
	}
	if (S_ISDIR(statBuf.st_mode))
	{
		mbDir = true;
		std::set<std::string>::iterator indexName = res.Index.begin();
		for (; indexName != res.Index.end(); ++indexName)
		{
			if (stat((mABSPath + (*indexName)).c_str(), &statBuf) == 0){
				mABSPath += *indexName;
				mbDir = false;
				mbFile = true;
				break;
			}
		}
		if (!mbFile){
			if (mbAutoIndex){
				mBody = getIndexListOf(mABSPath);
				mHeaderMap["Content-Type"] = "text/html";
			}
			else if (!mbAutoIndex)
				SetStatusOf(404);
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
		}
		else
			mCGIPath = "";
	}
	if (!mbCGI){
		std::ifstream ifs(mABSPath);
		createResponseBody();
	}

    // //--------------------------
    // mABSPath = res.ABSPath;
    // struct stat statBuf;
    // if (stat(mABSPath.c_str(), &statBuf) == -1)
    //     return false;
    // else if (S_ISDIR(statBuf.st_mode))
    // {
    //     mbDir = true;
    //     if (!res.BAutoIndex)
    //     {
    //         req.statusCode = 404;
    //         return (false);
    //     }
    // }
    // else if (S_ISREG(statBuf.st_mode))
    //     mbFile = true;
    // return (true);
    // // 만약 dir이고 autoIndex ON 이면, directroy list;
    // // autoIndex Off 이면 404
    // // 없는 파일이면 404
    // // 있는 파일이면 return
}

void Response::SetStatusOf(int statusCode)
{
    mStatCode = statusCode;
	if (mErrorPage.find(statusCode) != mErrorPage.end()){
		mABSPath = mErrorPage[statusCode];
		createResponseBody();
		// createResponseHeader();
	}
	else{
		mBody = StatusPage::GetInstance()->GetStatusPageOf(statusCode);
		// createResponseHeader();
	}
}

void Response::MakeResponse(struct Request& req)
{
    struct Resource res = ConfigHandler::GetConfigHandler().GetResource(req.port, req.domain, req.URI);

    mParams = req.params;
	setFromResource(res);
	setDate();
    if (req.statusCode || !isValidMethod(req, res)) // TODO: http ver, method, abs path
 	{
		SetStatusOf(req.statusCode);
        return ;
	}
        // TODO:
        // find server block using the request
	if (req.method == "GET")
		processGET(res);
	else if (req.method == "POST")
		processPOST(res);
	// else if (req.method == "HEAD")
	// 	processHEAD(res);
	// else if (req.method == "PUT")
	// 	processPUT(res);
	// else if (req.method == "DELETE")
	// 	processDELETE(res);
}

void Response::SetRequestBody(const std::string& requestBody)
{
	mRequestBody = requestBody;
}

std::string Response::GetRequestBody()
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
	createResponseHeader();
}


std::string Response::GetCGIPath() const
{
	return (mCGIPath);
}


// TODO: Body를 reference로 받아서 복사되지 않도록(오버헤드 이슈) 처리해야 함.
std::string Response::GenResponseMsg()
{
	std::string ret;
	createResponseHeader();
	ret = mStartLine;
	ret += mHeader;
    ret += mBody;
	std::cout << ret << std::endl;
	return ret;
}

void Response::setFromResource(struct Resource res)
{
	mbAutoIndex = res.BAutoIndex;
	mErrorPage = res.ErrorPage;
	mABSPath = res.ABSPath;
}

void Response::setDate()
{
    char buf[100];
    std::time_t time = std::time(NULL);
	// Date: [Day], [date] [Month] [year] [time] KST;
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", std::localtime(&time));
	mHeaderMap["Date"] = std::string(buf);
}

void Response::processPOST(struct Resource& res)
{
	struct stat statBuf;
	if (stat(mABSPath.c_str(), &statBuf) == -1)
	{
		SetStatusOf(204);
		return ;
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
		}
		else
			mCGIPath = "";
	}
	if (!mbCGI){
		SetStatusOf(204);
		return ;
    }
}

void Response::processDELETE()
{
	struct stat statBuf;
	if (stat(mABSPath.c_str(), &statBuf) == -1 || S_ISDIR(statBuf.st_mode))
	{
		SetStatusOf(204);
		return ;
	}
    mbFile = true;
    std::remove(mABSPath.c_str());
    mHeaderMap["Content-Type"] = "application/json";
	mBody = "{\n \"message\": \"Item deleted successfully.\"\n}";
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
		size_t idx = line.find(": ");
		if (line == "\r")
			break;
		else if (line[line.length() - 1] != '\r' || idx == std::string::npos)
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
		mBody = mBody.substr(mBody.find("\r\n\r\n") + 4);
	// std::cout << "body\n" << mBody;
}

const char* Response::GetABSPath() const
{
	return (mABSPath.c_str());
}

std::map<std::string, std::string> Response::GetParams() const
{
	return (mParams);
}
