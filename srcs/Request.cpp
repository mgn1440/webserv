#include "Request.hpp"

void initRequest(struct Request &request)
{
	request.StatusCode = 0;
	request.ParsedStatus = PARSED_NOT;
	request.StartLine = "";
	request.Method = "";
	request.URI = "";
	request.Domain = "";
	request.Port = 0;
	request.HTTPVersion = "";
	request.Headers.clear();
	request.Body = "";
	request.ConnectionStop = false;
	request.ChunkedStatus = false;
	request.ChunkedNum = 0;
	request.query = "";
}