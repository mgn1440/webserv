#ifndef HTTP_HPP
# define HTTP_HPP
# include <string>
# include <vector>
# include <sstream>
# include <map>
# include "ConfigHandler.hpp"

struct request
{
	int statusCode;
	int parsedStatus;
	std::string method;
	std::string uri;
	std::map<std::string, std::string> headers; // header is sorted?
	std::string body;
};

class Http
{
public:
	// occf
	Http(const ConfigHandler& ConfigHandler); // ConfFile is class;
	~Http();

	// interface
	void ReciveRequestMessage(const std::string& data);
	std::string GetResponseMessage(void);
private:
	// occf not used
	Http(const Http& src);
	Http& operator=(const Http& rhs);

	// member var
	std::string mRequestBuffer;
	struct request parsedRequest;
	size_t savedHeaderSize;
	int consumeBufferSize;
	const ConfigHandler& mConfigHandler;

	static const size_t maxStartLineSize = 8000; // get from conf_file
	static const size_t maxHeaderSize = 8000; // get from conf_file
	static const size_t maxBodySize = 8000; // get from conf_file

	// method
	void parseHttpRequest(void);
	void parseStartLine(std::istringstream& input);
	void parseHeader(std::istringstream& input);
	void parseBody(std::istringstream& input);
	void setHttpStatusCode(int statusCode);
	// HttpResponse httpResponse; not implemented


	// debug
	void	printParsedHttpRequest(const struct request& r);
};

# define START_PARSED 1
# define HEADER_PARSED 2
# define BODY_PARSED 4
# define ALL_PARSED 7

#endif
