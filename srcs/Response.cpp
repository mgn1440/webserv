#include <string>
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
    mCGI = rhs.mCGI;
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
    mCGI = rhs.mCGI;
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
    std::cout << req.statusCode << std::endl;
    if (req.statusCode != 200 || !isValidStartLine(req, res)) // TODO: http ver, method, abs path
    {
        mStatCode = req.statusCode;
        return ;
    }
    // TODO:
    // find server block using the request
    mContentType = ConfigHandler::GetConfigHandler().GetContentType(mABSPath);
    mCGI = mABSPath.substr(mABSPath.find_last_of(".") + 1);
    if (mbFile && res.CGI.find(mCGI) != res.CGI.end())
    {
        mbCGI = true;
        mCGI = res.CGI[mCGI];
    }
    else
        mCGI = "";
}

void Response::CreateResponseHeader()
{
    mStartLine = mHttpVer + " ";
    std::stringstream ss;
    ss << mStatCode;
    mStartLine += ss.str() + " " + "OK" +"\r\n"; // TODO: StatCode => mStat 으로 변환하는 코드가 있어야 함
    mHeader = "Date: " + mDate;
    mHeader += "Server: " + mServer;
    if (mContentType != "")
        mHeader += "Content-Type: " + mContentType;
    if (mbContentLen == true)
        mHeader += "Content-length: ";

}

void Response::CreateResponseBody()
{
    // if (!mbFile) // Directory
    // {
    //     //mBody += getIndexListOf(mABSPath);
    //     return ;
    // }
    // static file
    std::cout << mABSPath << std::endl;
    int fd = open(mABSPath.c_str(), O_RDONLY, 0755);
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


bool Response::isValidStartLine(struct Request& req, struct Resource& res)
{
    if (find(res.HttpMethod.begin(), res.HttpMethod.end(), req.method) == res.HttpMethod.end())
    {
        req.statusCode = 501;
        return (false);
    }
    std::cout << "ABS = " << res.ABSPath  << std::endl;
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