#ifndef HTTP_HPP
# define HTTP_HPP
# include <string>
# include <vector>
# include <sstream>
# include <map>
# include "Request.hpp"

class HttpRequest
{
public:
	// occf
	HttpRequest(int port); // ConfFile is class;
	~HttpRequest();
	HttpRequest& operator=(const HttpRequest& rhs);
	HttpRequest(const HttpRequest& src);
	HttpRequest();

	// interface
	std::vector<struct Request> ReceiveRequestMessage(const std::string& data);
private:
	// occf not used
	// member var
	int mPort;
	std::string mRequestBuffer;
	Request mParsedRequest;
	size_t mSavedHeaderSize;
	size_t mSavedBodySize;
	size_t mConsumeBufferSize;

	size_t maxStartLineSize;
	size_t maxHeaderSize;
	size_t maxBodySize;

	// method
	void parseHttpRequest(void);
	void parseStartLine(std::istringstream& input);
	void parseHeader(std::istringstream& input);
	void parseBody(std::istringstream& input);
	void parseContentLength(std::istringstream& input);
	void parseTransferEncoding(std::istringstream& input);
	void setHttpStatusCode(int statusCode);
	void getMaxSize(void);
	void splitStartLine(void);
	void parseURI(void);
	void CheckHTTP(std::string http);

	// process method
	void procHost(const std::string& fieldValue);
	// HttpResponse httpResponse; not implemented


	// debug
	void	printParsedHttpRequest(const struct Request& r);
};

#endif
