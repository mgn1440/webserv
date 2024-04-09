#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <ctime>
#include <sys/stat.h>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>

#include "ConfigHandler.hpp"
#include "Response.hpp"
#include "Resource.hpp"
#include "StatusPage.hpp"

//  TODO: Static이라고 가정 후 Response init을 돌린다.
//        추후 cgi를 돌릴 때 실제 ContentType과 ContentLength, status code 등을 설정한다.
std::string getIndexListOf(const std::string& path);

Response::Response()
{}

Response::~Response()
{}

Response::Response(const Response& rhs)
{
    mbCGI = rhs.mbCGI;
    mbAutoIndex = rhs.mbAutoIndex;
    mCGIPath = rhs.mCGIPath;
    mCGIExtension = rhs.mCGIExtension;
    mHeader = rhs.mHeader;
    mBody = rhs.mBody;
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
    mHeader = rhs.mHeader;
    mBody = rhs.mBody;
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

Response::Response(struct Request& req)
    : mbCGI(false)
    , mbAutoIndex(false)
    , mHttpVer("HTTP/1.1")
    , mServer("WebServ")
{
    struct Resource res = ConfigHandler::GetConfigHandler().GetResource(req.port, req.domain, req.URI);

    mbAutoIndex = res.BAutoIndex;
    
    char buf[100];
    std::time_t time = std::time(NULL);
	// Date: [Day], [date] [Month] [year] [time] GMT; IMF-fixdate
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", std::localtime(&time));
	mDate += std::string(buf);

    if (req.statusCode || !isValidMethod(req, res)) // TODO: http ver, method, abs path
        mStatCode = req.statusCode;               
    else{
        // TODO:
        // find server block using the request
        if (req.method == "GET")
            processGET(res);
        else if (req.method == "POST")
            processPOST(res);
        else if (req.method == "HEAD")
            processHEAD(res);
        else if (req.method == "PUT")
            processPUT(res);
        else if (req.method == "DELETE")
            processDELETE(res);
    }
}

void Response::CreateResponseHeader()
{
    mStartLine = mHttpVer + " ";
    std::stringstream ss;
    ss << mStatCode;
    mStartLine += ss.str() + " " + StatusPage::GetInstance()->GetStatusMessageOf(mStatCode) +"\r\n"; // TODO: StatCode => mStat 으로 변환하는 코드가 있어야 함
    mHeader = "Date: " + mDate;
    mHeader += "Server: " + mServer;
    if (mContentType != "")
        mHeader += "Content-Type: " + mContentType;
    if (mbContentLen == true)
        mHeader += "Content-length: ";
}

// TODO: directory가 들어왔을 때 autoindex on =>  directory listing page
// directory가 들어왔을 때 autoindex off => index file name이 붙어야 함
void Response::CreateResponseBody()
{
    // if (!mbFile) // Directory
    // {
    //     //mBody += getIndexListOf(mABSPath);
    //     return ;
    // }
    // static file
    std::string file = mABSPath + "test.txt"; // test
    int fd = open(file.c_str(), O_RDONLY, 0755); // test
    if (fd == -1)
        throw std::runtime_error("file open error");
    char buf[16384];
    ssize_t n;
    do
    {
        n = read(fd, buf, sizeof(buf)-1);
        std::cout << n << std::endl;
        buf[n] = '\0';
        mBody += std::string(buf);
    } while (n > 0);
    mbContentLen = true;
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
    mContentType = ConfigHandler::GetConfigHandler().GetContentType(mABSPath);
    mCGIExtension = mABSPath.substr(mABSPath.find_last_of(".") + 1);
    if (mbFile && res.CGIBinaryPath.find(mCGIExtension) != res.CGIBinaryPath.end())
    {
        mbCGI = true;
        mCGIPath = res.CGIBinaryPath[mCGIExtension];
    }
    else
        mCGIPath = "";
    

    //-=-------------------------
    mABSPath = res.ABSPath;
    struct stat statBuf;
    if (stat(mABSPath.c_str(), &statBuf) == -1)
        return false;
    else if (S_ISDIR(statBuf.st_mode))
    {
        mbDir = true;
        if (!res.BAutoIndex)
        {
            req.statusCode = 404;
            return (false);
        }
    }
    else if (S_ISREG(statBuf.st_mode))
        mbFile = true;
    return (true);
    // 만약 dir이고 autoIndex ON 이면, directroy list;
    // autoIndex Off 이면 404
    // 없는 파일이면 404
    // 있는 파일이면 return 
}

void Response::SetStatusOf(int statusCode)
{
    mStatCode = statusCode;
    mBody = StatusPage::GetInstance()->GetStatusPageOf(statusCode);
}