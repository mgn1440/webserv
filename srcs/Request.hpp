#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <map>
# include <vector>

struct Request
{
	int StatusCode;
	int ParsedStatus;
	std::string StartLine;
	std::string Method;
	std::string URI;
	std::string Domain;
	int Port;
	std::string HTTPVersion;
	std::map<std::string, std::string> Headers; // header is sorted?
	std::string Body;
	bool ConnectionStop;
	bool ChunkedStatus;
	size_t ChunkedNum;
	std::string query;
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
