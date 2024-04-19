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
	, mSavedBodySize(0)
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
	std::cout << "method: " << r.Method << std::endl;
	std::cout<< "uri: " << r.URI << std::endl;
	std::map<std::string, std::string>::const_iterator iter = r.Headers.begin();
	for (; iter != r.Headers.end(); ++iter)
		std::cout << iter->first << ": " << iter->second << std::endl;
	std::cout << std::endl;
}

std::deque<Response> HttpHandler::MakeResponseOf(const std::string& data)
{
	// 1. parse request;
	// 2. if condition is wrong make error code response
	// 3. if request is not end save not parsed string

	mRequestBuffer += data;
	// std::cout << "mRequestBuffer\n" << mRequestBuffer << std::endl;
	// TODO: Response에 body를 바로바로 저장하면 시간을 더욱 단축 시킬 수 있을 것
	std::deque<Response> ret;
	while (true)
	{
		parseHttpRequest();
		refreshBuffer(mRequestBuffer, mConsumeBufferSize);
		mConsumeBufferSize = 0;
		// std::cout << "parsedStatus: " << mParsedRequest.ParsedStatus << std::endl; // debug
		if (mParsedRequest.ParsedStatus != PARSED_ALL) 
			break;
		Response res;
		res.MakeResponse(mParsedRequest);
		// res.SetRequestBody(mParsedRequest.body);
		ret.push_back(res);
		initRequest(mParsedRequest);
		initHttpHandler();
	}
	return ret;
}

void HttpHandler::parseHttpRequest(void)
{
	std::istringstream inputStream(mRequestBuffer);
	if (mParsedRequest.ParsedStatus == PARSED_NOT)
		parseStartLine(inputStream);
	if (mParsedRequest.ParsedStatus == (PARSED_START))
		parseHeader(inputStream);
	if (mParsedRequest.ParsedStatus == (PARSED_START | PARSED_HEADER))
		parseBody(inputStream);
}


void HttpHandler::parseStartLine(std::istringstream& input)
{
	std::string buf;
	getline(input, buf);
	if (input.eof())
	{
		mReq = "";
		return;
	}
	mReq += buf + "\n"; // debug
	if (!checkCRLF(buf)){
		return setHttpStatusCode(400); // bad request
	}
	mConsumeBufferSize += buf.size() + 1;
	mParsedRequest.StartLine = buf;
	splitStartLine();
	mParsedRequest.ParsedStatus |= PARSED_START;
}

void HttpHandler::parseHeader(std::istringstream& input)
{
	std::string buf;
	std::string header;
	while (true)
	{
		getline(input, buf);
		if (input.eof())
			return;
		header += buf + "\n";
		mSavedHeaderSize += buf.size() + 1; // +1 = linefeed character size add;
		mConsumeBufferSize += buf.size() + 1;
		if (mSavedHeaderSize > MAX_HEDAER_SIZE){ // check header size
			mParsedRequest.ConnectionStop = true;
			return setHttpStatusCode(431); // Request Header Fields Too Large, connetction close
		}
		if (buf.size() == 1 && checkCRLF(buf)){ //header section is over
			if (!mParsedRequest.StatusCode)
				mParsedRequest.ParsedStatus |= PARSED_HEADER;
			break;
		}
		if (!checkCRLF(buf))
			setHttpStatusCode(400); // bad request
		setHeader(buf);
	}
	mReq += header; // debug
	std::cout << "\033[1;33m" << "~~Print Request~~\n" <<  mReq << "\033[0m" << "\n\n";
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
	mParsedRequest.Headers[fieldName] = fieldValue;
}

void HttpHandler::parseBody(std::istringstream& input)
{
	// std::cout << "parseBody" << std::endl; // debug
	if (mParsedRequest.Headers.find("Transfer-Encoding") != mParsedRequest.Headers.end()){

		// std::cout << "chunked" << std::endl; // debug
		if (mParsedRequest.Headers["Transfer-Encoding"] != "chunked"){
			mParsedRequest.ConnectionStop = true;
			setHttpStatusCode(400); // and connection cut
			return;
		}
		if (mParsedRequest.Headers.find("Content-Length") != mParsedRequest.Headers.end())
			mParsedRequest.Headers.erase("Content-Length");
		parseTransferEncoding(input);
	}
	else if (mParsedRequest.Headers.find("Content-Length") != mParsedRequest.Headers.end())
		parseContentLength(input);
	else
	{
		// TODO: GET, DELETE일 때 Body 제한?
		mParsedRequest.Headers["Content-Length"] = "0";
		mParsedRequest.ParsedStatus |= PARSED_BODY;
	}
}

void HttpHandler::parseContentLength(std::istringstream& input)
{
	// std::cout << "parseContentLength" << std::endl; // debug
	std::string body;
	size_t contentLength = 0;
	try{
		 contentLength = convertNum(mParsedRequest.Headers["Content-Length"]);
	}
	catch (std::exception& e){
		mParsedRequest.ParsedStatus |= PARSED_BODY;
		return setHttpStatusCode(400);
	}
	if (!contentLength){
		mParsedRequest.ParsedStatus |= PARSED_BODY;
		return;
	}
	if (contentLength > mMaxbodySize){
		mParsedRequest.ConnectionStop = true;
		mParsedRequest.ParsedStatus |= PARSED_BODY;
		return setHttpStatusCode(413);
	}
	int bufferSize = 1000;
	char buf[bufferSize];
	while (true)
	{
		input.read(buf, bufferSize);
		size_t cnt = input.gcount();
		mConsumeBufferSize += cnt;
		mParsedRequest.Body.append(buf, cnt);
		if (mParsedRequest.Body.size() > mMaxbodySize){
			mParsedRequest.ConnectionStop = true;
			mParsedRequest.ParsedStatus |= PARSED_BODY;
			return setHttpStatusCode(413); // content too large
		}
		// TODO: why cnt compare with contentLength?
		if (cnt != contentLength)
		{
			// std::cout << "cnt is 0, body size is " << mParsedRequest.body.size() << ", contentLength is " << contentLength << std::endl; // debug 
			break;
		}
	}
	if (mParsedRequest.Body.size() == contentLength)
		mParsedRequest.ParsedStatus |= PARSED_BODY;
}

void HttpHandler::parseTransferEncoding(std::istringstream& input)
{
	//decompress impossible;
	std::string size;
	while (true){
		if (!mParsedRequest.ChunkedStatus){ // before buffer where parsed?
			std::getline(input, size);
			if (input.eof())
				return;
			mConsumeBufferSize += size.size() + 1;
			trim(size, "\r");
			mParsedRequest.ChunkedNum = convertHex(size);
			mParsedRequest.ChunkedStatus = true;
			// std::cout << "num: " << mParsedRequest.ChunkedNum << std::endl; // debug
		}
		size_t num = mParsedRequest.ChunkedNum;
		if (input.str().size() - input.tellg() < num + 2)
		{
			// std::cout << "second chunk eof" << std::endl; // debug
			return ;
		}
		char *str = new char[num + 3];
		str[input.readsome(str, num + 2)] = '\0';
		// std::cout << "str: " << str << std::endl; // debug
		mSavedBodySize += num;
		mConsumeBufferSize += num + 2;
		if (mSavedBodySize > mMaxbodySize)
		{
			// std::cout << "maxSizeOver: " << mSavedBodySize << std::endl;
			mParsedRequest.ConnectionStop = true;
			// mParsedRequest.ParsedStatus |= PARSED_BODY;
			// return setHttpStatusCode(413); // content too large
			setHttpStatusCode(413); // content too large
		}
		else if (str[num] != '\r' || str[num + 1] != '\n')
		{
			// std::cout << "not \\r\\n: [" << (int)str[num] << "], [" << (int)str[num + 1] << "]" << input.str() <<std::endl;
			// mParsedRequest.ParsedStatus |= PARSED_BODY;
			// return setHttpStatusCode(400); // bad request
			setHttpStatusCode(400); // bad request
		}
		else
			mParsedRequest.Body += std::string(str, num);
		mParsedRequest.ChunkedNum = 0;
		mParsedRequest.ChunkedStatus = false;
		delete[] str;
		if (num == 0)
		{
			// std::cout << "num is 0" << std::endl; // debug
			break;
		}
		// else
		// 	std::cout << "num is not 0" << std::endl; // debug
	}
	mParsedRequest.ParsedStatus |= PARSED_BODY;
}

void HttpHandler::setHttpStatusCode(int statusCode)
{
	mParsedRequest.StatusCode = statusCode;
}

void HttpHandler::getMaxSize()
{
	mMaxbodySize = ConfigHandler::GetConfigHandler().GetMaxSize(mParsedRequest.Port, mParsedRequest.URI); // Redefinition config handler
}

void HttpHandler::splitStartLine()
{
	if (mParsedRequest.StartLine.size() + 1 > MAX_STARTLINE_SIZE){ // +1 = linefeed character size add{
		mParsedRequest.ParsedStatus ^= PARSED_START;
		return setHttpStatusCode(414); // size error occured
	}
	std::vector<std::string> startLine = split(mParsedRequest.StartLine, " ");
	mParsedRequest.Method = startLine[0];
	mParsedRequest.URI = startLine[1];
	parseURI();
	mParsedRequest.HTTPVersion = startLine[2];
	trim(mParsedRequest.HTTPVersion, "\r");
	checkHTTP(mParsedRequest.HTTPVersion);
}

void HttpHandler::parseURI()
{
	percentDecoding(mParsedRequest.URI);
	if (mParsedRequest.URI.front() == '/'){ // query parse logic not needed
		// size_t pos = mParsedRequest.URI.find_first_of("?");
		// if (pos == std::string::npos)
		// 	return;
		// else{ // query exist
		// 	std::string paramString = mParsedRequest.URI.substr(pos + 1);
		// 	std::vector<std::string> pramaVec = split(paramString, "&");
		// 	for (std::vector<std::string>::iterator it = pramaVec.begin(); it != pramaVec.end(); it++){
		// 		std::vector<std::string> param = split(*it, "=");
		// 		if (param.size() != 2) continue;
		// 		mParsedRequest.params[param[0]] = param[1];
		// 	}
		// 	mParsedRequest.URI = mParsedRequest.URI.substr(0, pos - 1);
		// }
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
	if (mParsedRequest.Headers.find("Referer") == mParsedRequest.Headers.end())
		return;
	std::string referer = mParsedRequest.Headers["Referer"];
	if (referer[referer.size() - 1] == '/')
		referer.erase(referer.size() - 1);
	mParsedRequest.URI = referer + mParsedRequest.URI;
	size_t pos = 0;
	for (int i = 0; i < 3; i++){
		pos = mParsedRequest.URI.find("/", pos);
		pos++;
	}
	mParsedRequest.URI = mParsedRequest.URI.substr(pos - 1);
}

void HttpHandler::procHost()
{
	if (mParsedRequest.Headers.find("Host") == mParsedRequest.Headers.end())
		return;
	std::vector<std::string> vec = split(mParsedRequest.Headers["Host"], ":");
	mParsedRequest.Domain = vec[0];
	mParsedRequest.Port = mPort;
	if (vec.size() == 2)
		mParsedRequest.Port = convertNum(vec[1]);
	getMaxSize();
}

void HttpHandler::initHttpHandler()
{
	mSavedHeaderSize = 0;
	mSavedBodySize = 0;
	mMaxbodySize = 0;
}