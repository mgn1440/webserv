#include "Request.hpp"

void initRequest(struct Request& request)
{
	request.statusCode = 0;
	request.parsedStatus = PARSED_NOT;
	request.startLine = "";
	request.method = "";
	request.URI = "";
	request.domain = "";
	request.port = 0;
	request.params.clear();
	request.HTTPVersion = "";
	request.headers.clear();
	request.body = "";
	request.connectionStop = false;
	request.chunkedStatus = false;
	request.chunkedNum = 0;
}