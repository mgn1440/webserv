#include <iostream>
#include <stdexcept>
#include <iomanip>
#include "../includes/Http.hpp"
#include "../includes/parseUtils.hpp"

Http::Http(const ConfigHandler& configHandler) // httpResponse(confFile)
	: mRequestBuffer()
	, savedHeaderSize()
	, consumeBufferSize()
	, mConfigHandler(configHandler)
{
	// memset(&parsedRequest, 0, sizeof(struct request)); // using memset error occured
	parsedRequest.parsedStatus = 0;
	parsedRequest.statusCode = 0;
}

Http::~Http()
{}

void	Http::printParsedHttpRequest(const struct request& r)
{
	std::cout << "method: " << r.method << std::endl;
	std::cout<< "uri: " << r.uri << std::endl;
	std::map<std::string, std::string>::const_iterator iter = r.headers.begin();
	for (; iter != r.headers.end(); ++iter)
		std::cout << iter->first << ": " << iter->second << std::endl;
	std::cout << std::endl;
}

void Http::ReciveRequestMessage(const std::string& data)
{
	// 1. parse request;
	// 2. if condition is wrong make error code response
	// 3. if request is not end save not parsed string

	mRequestBuffer += data;
	parseHttpRequest();
	// printParsedHttpRequest(parsedRequest); => debug function
	refreshBuffer(mRequestBuffer, consumeBufferSize);
}

std::string Http::GetResponseMessage(void)
{
	if (parsedRequest.parsedStatus == ALL_PARSED || parsedRequest.statusCode){
		//TODO: implemented httpResponse.makeResponse;
	}
	return (""); // temp
}

void Http::parseHttpRequest(void)
{
	try
	{
		std::istringstream inputStream(mRequestBuffer);
		if (!(parsedRequest.parsedStatus & START_PARSED))
			parseStartLine(inputStream);
		if (!(parsedRequest.parsedStatus & HEADER_PARSED))
			parseHeader(inputStream);
		if (!(parsedRequest.parsedStatus & BODY_PARSED))
			parseBody(inputStream);
	}
	catch (std::exception & e)
	{
		if (parsedRequest.statusCode != 0){
			//TODO: make response, if write section is check all right, if this will not needed
		}
	}
	// try catch not necessary??

	// 메시지를 정상적으로 parsing 했다면 parsedStauts를 초기화 해줘야 함
	parsedRequest.parsedStatus = 0;
}

void Http::parseStartLine(std::istringstream& input)
{
	std::string buf;
	getline(input, buf);
	if (!checkCRLF(buf)){
		if (input.peek() == std::istringstream::traits_type::eof()) // if next char is eof
			throw std::runtime_error("not full request");
		setHttpStatusCode(400); // bad request
	}
	if (buf.size() + 1 > static_cast<size_t>(maxStartLineSize)) // +1 = linefeed character size add
		setHttpStatusCode(414); // size error occured
	parsedRequest.parsedStatus |= START_PARSED;
	consumeBufferSize += buf.size() + 1;
	std::vector<std::string> startLine = split(buf, " ");
	parsedRequest.method = startLine[0];
	parsedRequest.uri = startLine[1];
	// startLine[2] -> http version not save
}

void Http::parseHeader(std::istringstream& input)
{
	std::string buf;
	while (true)
	{
		getline(input, buf);
		savedHeaderSize += buf.size() + 1; // +1 = linefeed character size add;
		consumeBufferSize += buf.size() + 1;
		if (savedHeaderSize > maxHeaderSize){ // check header size
			savedHeaderSize = 0;
			setHttpStatusCode(431); // Request Header Fields Too Large
		}
		if (buf.size() == 1 && checkCRLF(buf)) //header section is over
			break;
		if (!checkCRLF(buf)){
			if (input.peek() == std::istringstream::traits_type::eof()) // if next char is eof
				throw std::runtime_error("not full request");
			savedHeaderSize = 0;
			setHttpStatusCode(400); // bad request
		}
		int colon = buf.find_first_of(':');
		if (static_cast<const unsigned long>(colon) == std::string::npos){ // field syntax error
			savedHeaderSize = 0;
			setHttpStatusCode(400); // bad request
		}
		std::string fieldName = buf.substr(0, colon);
		if (fieldName.find_first_of(' ') != std::string::npos){ // field name error check
			savedHeaderSize = 0;
			setHttpStatusCode(400); // bad request
		}
		std::string fieldValue = buf.substr(colon + 1, buf.size() - 2);
		//TODO: check mesasage type and obs-fold, content type == message obs-fold can recieve
		fieldValue = trim(fieldValue); // del ows
		parsedRequest.headers[fieldName] = fieldValue;
	}
	savedHeaderSize = 0;
	parsedRequest.parsedStatus |= HEADER_PARSED;
}

void Http::parseBody(std::istringstream& input)
{
	// std::string body;
	// body = body.substr(content-length); // we are going to use content-length header or Transfer-Encoding: chunked
	// if convert to num is failed status code is 400 bad request
	std::string buf;
	// int buffersize = 100; // temp
	while (true)
	{
		// input >> std::setw(buffersize) >> buf; // using setw limit input size
		std::getline(input, buf);
		buf += "\n";
		consumeBufferSize += buf.size();
		parsedRequest.body += buf;
		if (parsedRequest.body.size() > maxBodySize)
			setHttpStatusCode(413); // content too large
		if (input.eof()){
			parsedRequest.parsedStatus |= BODY_PARSED;
			break;
		}
	}
}

void Http::setHttpStatusCode(int statusCode)
{
	parsedRequest.statusCode = statusCode;
	throw std::runtime_error("error occured"); // if try catch not necessary, then delete
}
