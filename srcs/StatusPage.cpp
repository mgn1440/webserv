#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include "StatusPage.hpp"

StatusPage* StatusPage::sInstance = NULL;

StatusPage* StatusPage::GetInstance()
{
	if (sInstance == NULL){
		sInstance = new StatusPage();
	}
	return sInstance;
}

StatusPage::StatusPage()
{
	initStatusCode();
}

StatusPage::~StatusPage()
{
}

std::string StatusPage::GetStatusPageOf(int statusCode)
{
    std::stringstream ss;
    ss << statusCode;
    std::string num = ss.str();

	std::string ret = "<!DOCTYPE html>";
	ret += "<body><h1>" + num + "</h1></body>";
	ret += "</html>";
	return ret;
}

void StatusPage::initStatusCode(void)
{
    mStatusMessage[100] = "Continue";
    mStatusMessage[101] = "Switching Protocols";
    mStatusMessage[102] = "Processing";
    mStatusMessage[200] = "OK";
    mStatusMessage[201] = "Created";
    mStatusMessage[202] = "Accepted";
    mStatusMessage[203] = "Non-Authoritative Information";
    mStatusMessage[204] = "No Content";
    mStatusMessage[205] = "Reset Content";
    mStatusMessage[206] = "Partial Content";
    mStatusMessage[207] = "Multi-Status";
    mStatusMessage[208] = "Already Reported";
    mStatusMessage[226] = "IM Used";
    mStatusMessage[300] = "Multiple Choices";
    mStatusMessage[301] = "Moved Permanently";
    mStatusMessage[302] = "Found";
    mStatusMessage[303] = "See Other";
    mStatusMessage[304] = "Not Modified";
    mStatusMessage[305] = "Use Proxy";
    mStatusMessage[307] = "Temporary Redirect";
    mStatusMessage[308] = "Permanent Redirect";
    mStatusMessage[400] = "Bad Request";
    mStatusMessage[401] = "Unauthorized";
    mStatusMessage[402] = "Payment Required";
    mStatusMessage[403] = "Forbidden";
    mStatusMessage[404] = "Not Found";
    mStatusMessage[405] = "Method Not Allowed";
    mStatusMessage[406] = "Not Acceptable";
    mStatusMessage[407] = "Proxy Authentication Required";
    mStatusMessage[408] = "Request Timeout";
    mStatusMessage[409] = "Conflict";
    mStatusMessage[410] = "Gone";
    mStatusMessage[411] = "Length Required";
    mStatusMessage[412] = "Precondition Failed";
    mStatusMessage[413] = "Payload Too Large";
    mStatusMessage[414] = "URI Too Long";
    mStatusMessage[415] = "Unsupported Media Type";
    mStatusMessage[416] = "Range Not Satisfiable";
    mStatusMessage[417] = "Expectation Failed";
    mStatusMessage[418] = "I'm a teapot";
    mStatusMessage[421] = "Misdirected Request";
    mStatusMessage[422] = "Unprocessable Entity";
    mStatusMessage[423] = "Locked";
    mStatusMessage[424] = "Failed Dependency";
    mStatusMessage[425] = "Too Early";
    mStatusMessage[426] = "Upgrade Required";
    mStatusMessage[428] = "Precondition Required";
    mStatusMessage[429] = "Too Many Requests";
    mStatusMessage[431] = "Request Header Fields Too Large";
    mStatusMessage[451] = "Unavailable For Legal Reasons";
    mStatusMessage[500] = "Internal Server Error";
    mStatusMessage[501] = "Not Implemented";
    mStatusMessage[502] = "Bad Gateway";
    mStatusMessage[503] = "Service Unavailable";
    mStatusMessage[504] = "Gateway Timeout";
    mStatusMessage[505] = "HTTP Version Not Supported";
    mStatusMessage[506] = "Variant Also Negotiates";
    mStatusMessage[507] = "Insufficient Storage";
    mStatusMessage[508] = "Loop Detected";
    mStatusMessage[510] = "Not Extended";
    mStatusMessage[511] = "Network Authentication Required";
}

std::string StatusPage::GetStatusMessageOf(int statusCode)
{
	return (mStatusMessage[statusCode]);
}