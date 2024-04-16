#include <iostream>
#include "HttpHandler.hpp"

void HttpHandler::TestMethod()
{
	std::istringstream iss("a\r\n1234567890\r\nb\r\n");
	
	parseTransferEncoding(iss);
	iss.str("hello world\r\n0\r\n\r\n");
	iss.clear();
	parseTransferEncoding(iss);
	std::cout << "[" << mParsedRequest.body <<  "]" << std::endl;
}

int main()
{
	HttpHandler httpHandler(80);
	
	httpHandler.TestMethod();
}