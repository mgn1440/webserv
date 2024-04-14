#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <map>
# include <vector>

struct Request
{
	int statusCode;
	int parsedStatus;
	std::string startLine;
	std::string method;
	std::string URI;
	std::string domain;
	int port;
	std::map<std::string, std::string> params;
	std::string HTTPVersion;
	std::map<std::string, std::string> headers; // header is sorted?
	std::string body;
	bool connectionStop;
};

enum eParsedFlags
{
	PARSED_NOT = 0,
	PARSED_START = 1,
	PARSED_HEADER = 2,
	PARSED_BODY = 4,
	PARSED_ALL = 7
};

void initRequest(struct Request& request);

#endif
