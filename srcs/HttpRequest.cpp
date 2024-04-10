#include <iostream>
#include <stdexcept>
#include <iomanip>
#include "HttpRequest.hpp"
#include "parseUtils.hpp"
#include "convertUtils.hpp"
#include "Request.hpp"

HttpRequest::HttpRequest(int port)
	: mPort(port)
	, mRequestBuffer()
	, mParsedRequest()
	, mSavedHeaderSize()
	, mSavedBodySize()
	, mConsumeBufferSize()
	, maxStartLineSize(-1)
	, maxHeaderSize(-1)
	, maxBodySize(-1)
{
	initRequest(mParsedRequest);
}

HttpRequest::HttpRequest()
{
	std::cout << "hi\n";
}

HttpRequest::~HttpRequest()
{}

HttpRequest& HttpRequest::operator=(const HttpRequest& rhs)
{
	if (this == &rhs)
		return (*this);
	mPort = rhs.mPort;
	mRequestBuffer = rhs.mRequestBuffer;
	mParsedRequest = rhs.mParsedRequest;
	mSavedHeaderSize = rhs.mSavedHeaderSize;
	mSavedBodySize = rhs.mSavedBodySize;
	mConsumeBufferSize = rhs.mConsumeBufferSize;
	maxStartLineSize = rhs.maxStartLineSize;
	maxHeaderSize = rhs.maxHeaderSize;
	maxBodySize = rhs.maxBodySize;
	return (*this);
}

HttpRequest::HttpRequest(const HttpRequest& rhs)
{
	mPort = rhs.mPort;
	mRequestBuffer = rhs.mRequestBuffer;
	mParsedRequest = rhs.mParsedRequest;
	mSavedHeaderSize = rhs.mSavedHeaderSize;
	mSavedBodySize = rhs.mSavedBodySize;
	mConsumeBufferSize = rhs.mConsumeBufferSize;
	maxStartLineSize = rhs.maxStartLineSize;
	maxHeaderSize = rhs.maxHeaderSize;
	maxBodySize = rhs.maxBodySize;
}

void	HttpRequest::printParsedHttpRequest(const struct Request& r)
{
	std::cout << "method: " << r.method << std::endl;
	std::cout<< "uri: " << r.URI << std::endl;
	std::map<std::string, std::string>::const_iterator iter = r.headers.begin();
	for (; iter != r.headers.end(); ++iter)
		std::cout << iter->first << ": " << iter->second << std::endl;
	std::cout << std::endl;
}

std::vector<struct Request> HttpRequest::ReceiveRequestMessage(const std::string& data)
{
	// 1. parse request;
	// 2. if condition is wrong make error code response
	// 3. if request is not end save not parsed string

	mRequestBuffer += data;
	std::vector<struct Request> ret;
	while (true){
		parseHttpRequest();
		refreshBuffer(mRequestBuffer, mConsumeBufferSize);
		mConsumeBufferSize = 0;
		ret.push_back(mParsedRequest);
		initRequest(mParsedRequest);
		if (mParsedRequest.parsedStatus != PARSED_ALL)
			break;
	}
	return ret;
}

void HttpRequest::parseHttpRequest(void)
{
	std::istringstream inputStream(mRequestBuffer);
	if (mParsedRequest.parsedStatus == PARSED_NOT)
		parseStartLine(inputStream);
	if (mParsedRequest.parsedStatus == (PARSED_START))
		parseHeader(inputStream);
	if (mParsedRequest.parsedStatus == (PARSED_START | PARSED_HEADER))
		parseBody(inputStream);
}


void HttpRequest::parseStartLine(std::istringstream& input)
{
	std::string buf;
	getline(input, buf);
	if (!checkCRLF(buf)){
		if (input.peek() == std::istringstream::traits_type::eof()) // if next char is eof
			return;
		return setHttpStatusCode(400); // bad request
	}
	mParsedRequest.parsedStatus |= PARSED_START;
	mConsumeBufferSize += buf.size() + 1;
	mParsedRequest.startLine = buf;
}

void HttpRequest::parseHeader(std::istringstream& input) // TODO: savedHeaderSize를 초기화 하는 부분에서 refactoring 필요함
{
	std::string buf;
	while (true)
	{
		getline(input, buf);
		mSavedHeaderSize += buf.size() + 1; // +1 = linefeed character size add;
		mConsumeBufferSize += buf.size() + 1;
		if (mParsedRequest.hostParsed){ // host parsed?
			if (mSavedHeaderSize > maxHeaderSize){ // check header size
				mParsedRequest.connectionStop = true;
				return setHttpStatusCode(431); // Request Header Fields Too Large, connetction close
			}
		}
		if (buf.size() == 1 && checkCRLF(buf)){ //header section is over
			if (!mParsedRequest.statusCode)
				mParsedRequest.parsedStatus |= PARSED_HEADER;
			break;
		}
		if (!checkCRLF(buf)){
			if (input.peek() == std::istringstream::traits_type::eof()) // if next char is eof
				return
			setHttpStatusCode(400); // bad request
		}
		int colon = buf.find_first_of(':');
		if (static_cast<const unsigned long>(colon) == std::string::npos){ // field syntax error
			setHttpStatusCode(400); // bad request
		}
		std::string fieldName = buf.substr(0, colon);
		if (fieldName.find_first_of(' ') != std::string::npos){ // field name error check mSavedHeaderSize = 0;
			setHttpStatusCode(400); // bad request
		}
		std::string fieldValue = buf.substr(colon + 1, buf.size() - 2);
		//TODO: check mesasage type and obs-fold, content type == message obs-fold can recieve
		trim(fieldValue, " \r"); // del ows
		// field check -- map<std::string, function pointer>
		if (fieldName == "Host")
			procHost(fieldValue);
		mParsedRequest.headers[fieldName] = fieldValue;
	}
}

void HttpRequest::parseBody(std::istringstream& input)
{
	if (mParsedRequest.headers.find("Transfer-Encoding") != mParsedRequest.headers.end()){
		if (mParsedRequest.headers["Transfer-Encoding"] != "chunked"){
			mParsedRequest.connectionStop = true;
			return setHttpStatusCode(400); // and connection cut
		}
		if (mParsedRequest.headers.find("Content-Length") != mParsedRequest.headers.end())
			mParsedRequest.headers.erase("Content-Length");
		parseTransferEncoding(input);
	}
	else
		parseContentLength(input);
}

void HttpRequest::parseContentLength(std::istringstream& input)
{
	std::string body;
	size_t contentLength = 0;
	try{
		 contentLength = convertNum(mParsedRequest.headers["Content-Length"]);
	}
	catch (std::exception& e){
		return setHttpStatusCode(400);
	}
	if (contentLength > maxBodySize){
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
		if (mParsedRequest.body.size() > maxBodySize){
			mParsedRequest.connectionStop = true;
			return setHttpStatusCode(413); // content too large
		}
		if (cnt != contentLength)
			break;
	}
	if (mParsedRequest.body.size() == contentLength){
		mParsedRequest.parsedStatus |= PARSED_BODY;
	}
}

void HttpRequest::parseTransferEncoding(std::istringstream& input)
{
	//decompress impossible;
	std::string str;
	size_t bodySize = 0;
	while (true){
		std::getline(input, str);
		bodySize += str.size() + 1;
		trim(str, " \r\n");
		size_t num = convertHex(str);
		if (num == 0) break;
		std::getline(input, str);
		bodySize += str.size() + 1;
		if (bodySize > maxBodySize){
			mParsedRequest.connectionStop = true;
			return setHttpStatusCode(413); // content too large
		}
		trim(str, "\r");
		if (str.size() != num)
			return setHttpStatusCode(400); // bad request
		mParsedRequest.body += str;
	}
	mParsedRequest.statusCode |= PARSED_BODY;
}

void HttpRequest::setHttpStatusCode(int statusCode)
{
	mParsedRequest.statusCode = statusCode;
}

void HttpRequest::procHost(const std::string& fieldValue)
{
	std::vector<std::string> vec = split(fieldValue, ":");
	mParsedRequest.domain = vec[0];
	mParsedRequest.port = mPort;
	if (vec.size() == 2)
		mParsedRequest.port = convertNum(vec[1]);
	getMaxSize();
	splitStartLine();
	mParsedRequest.hostParsed = true;
}

void HttpRequest::getMaxSize()
{
	// int size[3];
	//ConfigHandler* cf = singletone::GetInsatnce();
	//size = cf.getMaxSizeOf();
	// maxStartLineSize = size[0];
	// maxHeaderSize = size[1];
	// maxBodySize = size[2];
}

void HttpRequest::splitStartLine()
{
	if (mParsedRequest.startLine.size() + 1 > maxStartLineSize){ // +1 = linefeed character size add{
		mParsedRequest.parsedStatus ^= PARSED_START;
		return setHttpStatusCode(414); // size error occured
	}
	std::vector<std::string> startLine = split(mParsedRequest.startLine, " ");
	mParsedRequest.method = startLine[0];
	mParsedRequest.URI = startLine[1];
	parseURI();
	mParsedRequest.HTTPVersion = startLine[2];
	trim(mParsedRequest.HTTPVersion, "\r");
	CheckHTTP(mParsedRequest.HTTPVersion);
}

void HttpRequest::parseURI()
{
	percentDecoding(mParsedRequest.URI);
	if (mParsedRequest.URI.front() == '/'){ // if origin-form
		size_t pos = mParsedRequest.URI.find_first_of("?");
		if (pos == std::string::npos){
			return;
		}
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

void HttpRequest::CheckHTTP(std::string http)
{
	http = http.substr(http.find("/") + 1);
	std::vector<std::string> nums = split(http, ".");
	if (nums.size() != 2)
		return setHttpStatusCode(505); // HttpRequest version not supported
	int major = convertNum(nums[0]);
	int minor = convertNum(nums[1]);
	if (major != 1 || minor != 1)
		return setHttpStatusCode(505); // HttpRequest version not supported
}
