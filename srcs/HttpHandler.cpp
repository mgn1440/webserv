#include <iostream>
#include <stdexcept>
#include <iomanip>
#include "HttpHandler.hpp"
#include "parseUtils.hpp"
#include "convertUtils.hpp"
#include "ConfigHandler.hpp"
#include "Response.hpp"
#include "STLUtils.hpp"

#define MAX_STARTLINE_SIZE 8000
#define MAX_HEDAER_SIZE 8000

HttpHandler::HttpHandler(int port)
	: mPort(port)
	, mRequestBuffer()
	, mParsedRequest()
	, mSavedHeaderSize()
	, mSavedBodySize()
	, mConsumeBufferSize()
	, mMaxbodySize(-1)
{
	initRequest(mParsedRequest);
}

HttpHandler::HttpHandler()
{
	std::cout << "hi\n";
}

HttpHandler::~HttpHandler()
{}

HttpHandler& HttpHandler::operator=(const HttpHandler& rhs)
{
	if (this == &rhs)
		return (*this);
	mPort = rhs.mPort;
	mRequestBuffer = rhs.mRequestBuffer;
	mParsedRequest = rhs.mParsedRequest;
	mSavedHeaderSize = rhs.mSavedHeaderSize;
	mSavedBodySize = rhs.mSavedBodySize;
	mConsumeBufferSize = rhs.mConsumeBufferSize;
	mMaxbodySize = rhs.mMaxbodySize;
	return (*this);
}

HttpHandler::HttpHandler(const HttpHandler& rhs)
{
	mPort = rhs.mPort;
	mRequestBuffer = rhs.mRequestBuffer;
	mParsedRequest = rhs.mParsedRequest;
	mSavedHeaderSize = rhs.mSavedHeaderSize;
	mSavedBodySize = rhs.mSavedBodySize;
	mConsumeBufferSize = rhs.mConsumeBufferSize;
	mMaxbodySize = rhs.mMaxbodySize;
}

void	HttpHandler::printParsedHttpRequest(const struct Request& r)
{
	std::cout << "method: " << r.method << std::endl;
	std::cout<< "uri: " << r.URI << std::endl;
	std::map<std::string, std::string>::const_iterator iter = r.headers.begin();
	for (; iter != r.headers.end(); ++iter)
		std::cout << iter->first << ": " << iter->second << std::endl;
	std::cout << std::endl;
}

std::deque<Response> HttpHandler::MakeResponseOf(const std::string& data)
{
	// 1. parse request;
	// 2. if condition is wrong make error code response
	// 3. if request is not end save not parsed string

	mRequestBuffer += data;
	std::cout << "mRequestBuffer\n" << mRequestBuffer << std::endl;
	std::deque<Response> ret;
	while (true)
	{
		parseHttpRequest();
		refreshBuffer(mRequestBuffer, mConsumeBufferSize);
		mConsumeBufferSize = 0;
		std::cout << "parsedStatus: " << mParsedRequest.parsedStatus << std::endl; // debug
		if (mParsedRequest.parsedStatus != PARSED_ALL)
			break;
		Response res;
		res.MakeResponse(mParsedRequest);
		res.SetRequestBody(mParsedRequest.body);
		ret.push_back(res);
		initRequest(mParsedRequest);
	}
	return ret;
}

void HttpHandler::parseHttpRequest(void)
{
	std::istringstream inputStream(mRequestBuffer);
	if (mParsedRequest.parsedStatus == PARSED_NOT)
		parseStartLine(inputStream);
	if (mParsedRequest.parsedStatus == (PARSED_START))
		parseHeader(inputStream);
	if (mParsedRequest.parsedStatus == (PARSED_START | PARSED_HEADER))
		parseBody(inputStream);
}


void HttpHandler::parseStartLine(std::istringstream& input)
{
	std::string buf;
	getline(input, buf);
	std::cout << "StartLine buf:" << buf <<  ", buffer: " << mRequestBuffer << std::endl; // debug
	if (input.eof())
		return;
	std::cout << "line buf length is " << buf.length() << std::endl;
	if (!checkCRLF(buf)){
		return setHttpStatusCode(400); // bad request
	}
	std::cout << "set PARSED_START" << std::endl; // debug
	mConsumeBufferSize += buf.size() + 1;
	mParsedRequest.startLine = buf;
	splitStartLine();
	mParsedRequest.parsedStatus |= PARSED_START;
}

void HttpHandler::parseHeader(std::istringstream& input)
{
	std::string buf;
	while (true)
	{
		getline(input, buf);
		if (input.eof())
			return;
		mSavedHeaderSize += buf.size() + 1; // +1 = linefeed character size add;
		mConsumeBufferSize += buf.size() + 1;
		if (mSavedHeaderSize > MAX_HEDAER_SIZE){ // check header size
			mParsedRequest.connectionStop = true;
			return setHttpStatusCode(431); // Request Header Fields Too Large, connetction close
		}
		if (buf.size() == 1 && checkCRLF(buf)){ //header section is over
			if (!mParsedRequest.statusCode)
				mParsedRequest.parsedStatus |= PARSED_HEADER;
			break;
		}
		if (!checkCRLF(buf))
			setHttpStatusCode(400); // bad request
		setHeader(buf);
	}
	procHost();
	procReferer();
}

void HttpHandler::setHeader(const std::string& str)
{
	int colon = str.find_first_of(':');
	if (static_cast<const unsigned long>(colon) == std::string::npos) // field syntax error
		setHttpStatusCode(400);										  // bad request
	std::string fieldName = str.substr(0, colon);
	if (fieldName.find_first_of(' ') != std::string::npos) // field name error check mSavedHeaderSize = 0;
		setHttpStatusCode(400);							   // bad request
	std::string fieldValue = str.substr(colon + 1, str.size() - 2);
	// TODO: check mesasage type and obs-fold, content type == message obs-fold can recieve
	trim(fieldValue, " \r"); // del ows
	// field check -- map<std::string, function pointer>
	mParsedRequest.headers[fieldName] = fieldValue;
}

void HttpHandler::parseBody(std::istringstream& input)
{
	// std::cout << "parseBody" << std::endl; // debug
	if (mParsedRequest.headers.find("Transfer-Encoding") != mParsedRequest.headers.end()){

		// std::cout << "chunked" << std::endl; // debug
		if (mParsedRequest.headers["Transfer-Encoding"] != "chunked"){
			mParsedRequest.connectionStop = true;
			setHttpStatusCode(400); // and connection cut
			return;
		}
		if (mParsedRequest.headers.find("Content-Length") != mParsedRequest.headers.end())
			mParsedRequest.headers.erase("Content-Length");
		parseTransferEncoding(input);
	}
	else if (mParsedRequest.headers.find("Content-Length") != mParsedRequest.headers.end())
		parseContentLength(input);
	else
	{
		// TODO: GET, DELETE일 때 Body 제한?
		mParsedRequest.headers["Content-Length"] = "0";
		mParsedRequest.parsedStatus |= PARSED_BODY;
	}
}

void HttpHandler::parseContentLength(std::istringstream& input)
{
	// std::cout << "parseContentLength" << std::endl; // debug
	std::string body;
	size_t contentLength = 0;
	try{
		 contentLength = convertNum(mParsedRequest.headers["Content-Length"]);
	}
	catch (std::exception& e){
		return setHttpStatusCode(400);
	}
	if (!contentLength){
		mParsedRequest.parsedStatus |= PARSED_BODY;
		return;
	}
	if (contentLength > mMaxbodySize){
		mParsedRequest.connectionStop = true;
		return setHttpStatusCode(413);
	}
	int bufferSize = 100;
	char buf[bufferSize];
	while (true)
	{
		input.read(buf, bufferSize);
		size_t cnt = input.gcount();
		mConsumeBufferSize += cnt;
		mParsedRequest.body.append(buf, cnt);
		if (mParsedRequest.body.size() > mMaxbodySize){
			mParsedRequest.connectionStop = true;
			return setHttpStatusCode(413); // content too large
		}
		// TODO: why cnt compare with contentLength?
		if (cnt != contentLength)
		{
			// std::cout << "cnt is 0, body size is " << mParsedRequest.body.size() << ", contentLength is " << contentLength << std::endl; // debug 
			break;
		}
	}
	if (mParsedRequest.body.size() == contentLength)
		mParsedRequest.parsedStatus |= PARSED_BODY;
}

void HttpHandler::parseTransferEncoding(std::istringstream& input)
{
	//decompress impossible;
	std::string size;
	while (true){
		if (!mParsedRequest.chunkedStatus){ // before buffer where parsed?
			std::getline(input, size);
			if (input.eof())
				return;
			mSavedBodySize += size.size() + 1;
			mConsumeBufferSize += size.size() + 1;
			trim(size, "\r");
			mParsedRequest.chunkedNum = convertHex(size);
			mParsedRequest.chunkedStatus = true;
			std::cout << "num: " << mParsedRequest.chunkedNum << std::endl; // debug
		}
		size_t num = mParsedRequest.chunkedNum;
		if (input.str().size() - input.tellg() < num + 2)
		{
			std::cout << "second chunk eof" << std::endl; // debug
			return ;
		}
		char *str = new char[num + 3];
		str[input.readsome(str, num + 2)] = '\0';
		std::cout << "str: " << str << std::endl; // debug
		mSavedBodySize += num + 2;
		mConsumeBufferSize += num + 2;
		if (mSavedBodySize > mMaxbodySize)
		{
			std::cout << "maxSizeOver: " << mSavedBodySize << std::endl;
			mParsedRequest.connectionStop = true;
			return setHttpStatusCode(413); // content too large
		}
		if (str[num] != '\r' || str[num + 1] != '\n')
		{
			std::cout << "not \\r\\n: [" << (int)str[num] << "], [" << (int)str[num + 1] << "]" << input.str() <<std::endl;
			return setHttpStatusCode(400); // bad request
		}
		mParsedRequest.body += std::string(str, num);
		mParsedRequest.chunkedNum = 0;
		mParsedRequest.chunkedStatus = false;
		if (num == 0)
		{
			std::cout << "num is 0" << std::endl; // debug
			break;
		}
		else
			std::cout << "num is not 0" << std::endl; // debug
	}
	mParsedRequest.parsedStatus |= PARSED_BODY;
}

void HttpHandler::setHttpStatusCode(int statusCode)
{
	mParsedRequest.statusCode = statusCode;
}

void HttpHandler::getMaxSize()
{
	// mMaxbodySize = ConfigHandler::GetConfigHandler().GetMaxSizes(); // Redefinition config handler
}

void HttpHandler::splitStartLine()
{
	if (mParsedRequest.startLine.size() + 1 > MAX_STARTLINE_SIZE){ // +1 = linefeed character size add{
		mParsedRequest.parsedStatus ^= PARSED_START;
		return setHttpStatusCode(414); // size error occured
	}
	std::vector<std::string> startLine = split(mParsedRequest.startLine, " ");
	mParsedRequest.method = startLine[0];
	mParsedRequest.URI = startLine[1];
	parseURI();
	mParsedRequest.HTTPVersion = startLine[2];
	trim(mParsedRequest.HTTPVersion, "\r");
	checkHTTP(mParsedRequest.HTTPVersion);
}

void HttpHandler::parseURI()
{
	percentDecoding(mParsedRequest.URI);
	if (mParsedRequest.URI.front() == '/'){ // if origin-form
		size_t pos = mParsedRequest.URI.find_first_of("?");
		if (pos == std::string::npos)
			return;
		else{ // query exist
			std::string paramString = mParsedRequest.URI.substr(pos + 1);
			std::vector<std::string> pramaVec = split(paramString, "&");
			for (std::vector<std::string>::iterator it = pramaVec.begin(); it != pramaVec.end(); it++){
				std::vector<std::string> param = split(*it, "=");
				if (param.size() != 2) continue;
				mParsedRequest.params[param[0]] = param[1];
			}
			mParsedRequest.URI = mParsedRequest.URI.substr(0, pos - 1);
		}
	}
	else{ // if absolute-form
		size_t endScheme = mParsedRequest.URI.find("://") + 3;
		mParsedRequest.URI = mParsedRequest.URI.substr(endScheme, mParsedRequest.URI.size() - endScheme);
	}
}

void HttpHandler::checkHTTP(std::string http)
{
	http = http.substr(http.find("/") + 1);
	std::vector<std::string> nums = split(http, ".");
	if (nums.size() != 2)
		return setHttpStatusCode(505); // Http version not supported
	int major = convertNum(nums[0]);
	int minor = convertNum(nums[1]);
	if (major != 1 || minor != 1)
		return setHttpStatusCode(505); // Http version not supported
}

void HttpHandler::procReferer()
{
	if (mParsedRequest.headers.find("Referer") == mParsedRequest.headers.end())
		return;
	mParsedRequest.URI = mParsedRequest.headers["Referer"] + mParsedRequest.URI;
	size_t pos = 0;
	for (int i = 0; i < 3; i++){
		pos = mParsedRequest.URI.find("/", pos);
		pos++;
	}
	mParsedRequest.URI = mParsedRequest.URI.substr(pos);
}

void HttpHandler::procHost()
{
	if (mParsedRequest.headers.find("Host") == mParsedRequest.headers.end())
		return;
	std::vector<std::string> vec = split(mParsedRequest.headers["Host"], ":");
	mParsedRequest.domain = vec[0];
	mParsedRequest.port = mPort;
	if (vec.size() == 2)
		mParsedRequest.port = convertNum(vec[1]);
	getMaxSize();
}
