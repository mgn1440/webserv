#ifndef HTTPHANDLER_HPP
# define HTTPHANDLER_HPP
# include <string>
# include <vector>
# include <deque>
# include <sstream>
# include <map>
# include "Request.hpp"
# include "Response.hpp"

class HttpHandler
{
public:
	HttpHandler(int port);
	~HttpHandler();
	HttpHandler& operator=(const HttpHandler& rhs);
	HttpHandler(const HttpHandler& src);
	HttpHandler();

	std::deque<Response> MakeResponseOf(const std::string& data);
private:
	int mPort;
	std::string mRequestBuffer;
	Request mParsedRequest;
	size_t mSavedHeaderSize;
	size_t mSavedBodySize;
	size_t mConsumeBufferSize;
	size_t mMaxbodySize;

	void parseHttpRequest(void);
	void parseStartLine(std::istringstream& input);
	void parseHeader(std::istringstream& input);
	void setHeader(const std::string& str);
	void parseBody(std::istringstream& input);
	void parseContentLength(std::istringstream& input);
	void parseTransferEncoding(std::istringstream& input);
	void setHttpStatusCode(int statusCode);
	void getMaxSize(void);
	void splitStartLine(void);
	void parseURI(void);
	void checkHTTP(std::string http);
	void initHttpHandler();

	void procHost();
	void procReferer();
	void procConnection();

	// debug
	void	printParsedHttpRequest(const struct Request& r);
	std::string mReq;
};

#endif
