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
    mDate = rhs.mDate;
    mServer = rhs.mServer;
    mbContentLen = rhs.mbContentLen;
    mContentType = rhs.mContentType;
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
    mDate = rhs.mDate;
    mServer = rhs.mServer;
    mbContentLen = rhs.mbContentLen;
    mContentType = rhs.mContentType;
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
    , mABSPath()
    , mDate()
    , mServer("WebServ")
    , mbContentLen()
    , mContentType("text/html")
{}

void Response::CreateResponseHeader()
{
    mbContentLen = true; // debug
    mStartLine = mHttpVer + " ";
    mStartLine += intToString(mStatCode) + " " + StatusPage::GetInstance()->GetStatusMessageOf(mStatCode) +"\r\n";
    mHeader = "Date: " + mDate + "\r\n";
    mHeader += "Server: " + mServer + "\r\n";
    if (mContentType != "")
        mHeader += "Content-Type: " + mContentType + "\r\n";
    if (mbContentLen == true)
        mHeader += "Content-length: " + intToString(mBody.size()) + "\r\n";
	mHeader += "\r\n";
}

// TODO: directory가 들어왔을 때 autoindex on =>  directory listing page
// directory가 들어왔을 때 autoindex off => index file name이 붙어야 함
void Response::CreateResponseBody()
{
	std::ifstream ifs(mABSPath);
    if (ifs.fail())
        throw std::runtime_error("file open error");
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
	// Server: webserv
	ret += "Server: " + mServer + "\r\n";
	ret += "Date: " + mDate + "\r\n\r\n";
    CreateResponseBody();
    ret += mBody;
    std::cout << mBody << std::endl;
    std::cout << ret << std::endl;
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
				mContentType = "text/html";
			}
			else if (!mbAutoIndex)
				SetStatusOf(404);
		}
	}
	else
		mbFile = true;
	if (mbFile){
		mContentType = ConfigHandler::GetConfigHandler().GetContentType(mABSPath);
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
		CreateResponseBody();
		CreateResponseHeader(); // doing this
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
		CreateResponseBody();
		CreateResponseHeader();
	}
	else{
		mBody = StatusPage::GetInstance()->GetStatusPageOf(statusCode);
		CreateResponseHeader();
	}
}

void Response::MakeResponse(struct Request& req)
{
    struct Resource res = ConfigHandler::GetConfigHandler().GetResource(req.port, req.domain, req.URI);

    mParams = req.params;
	setFromResource(res);
    char buf[100];
    std::time_t time = std::time(NULL);
	// Date: [Day], [date] [Month] [year] [time] GMT; IMF-fixdate
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", std::localtime(&time));
	mDate += std::string(buf);

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


bool Response::IsCGI()
{
	return mbCGI;
}

void Response::SetCGIBody(const std::string& CGIBody)
{
	mBody = CGIBody;
	CreateResponseHeader();
}


// TODO: Body를 reference로 받아서 복사되지 않도록(오버헤드 이슈) 처리해야 함.
std::string Response::GenResponseMsg()
{
	std::string ret;
	ret = mStartLine;
	ret += mHeader;
    ret += mBody;
	return ret;
}

void Response::setFromResource(struct Resource res)
{
	mbAutoIndex = res.BAutoIndex;
	mErrorPage = res.ErrorPage;
	mABSPath = res.ABSPath;
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
		mContentType = ConfigHandler::GetConfigHandler().GetContentType(mABSPath);
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
    mContentType = "application/json";
	mBody = "{\n \"message\": \"Item deleted successfully.\"\n}";
}
