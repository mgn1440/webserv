#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include "StatusPage.hpp"

StatusPage* StatusPage::sInstance = NULL;

StatusPage* StatusPage::GetInstance()
{
	if (sInstance == NULL){
		sInstance == new StatusPage();
	}
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

	std::string ret = "<!DOCTYPE html>\n";
	ret += "<html lang=\"en\">\n";
	ret += "<head>\n";
	ret += "<meta charset=\"UTF-8\">\n";
	ret += "<title>";
	ret += num;
	ret += ":" + GetStatusMessageOf(statusCode);
	ret += "</title>\n";
	ret += "<div id=\"black\">\n";
	ret += "<div id=\"dvd\">";
	ret += num;
	ret += "<br>";
	ret += GetStatusMessageOf(statusCode);
	ret += "</div>\n";
	ret += "</div>\n";
	ret += "<style>\n";
	ret += "body {\n";
	ret += "margin: 0;\n";
	ret += "box-sizing: border-box;\n";
	ret += "overflow: hidden;\n";
	ret += "}\n";
	ret += "#black {\n";
	ret += "height: 100vh;\n";
	ret += "width: 100vw;\n";
	ret += "background-color: #111;\n";
	ret += "}\n";
	ret += "#dvd {\n";
	ret += "display: inline-block;\n";
	ret += "position: absolute;\n";
	ret += "max-width: 100%;\n";
	ret += "background-repeat: no-repeat;\n";
	ret += "background-position: center;\n";
	ret += "font-weight: bolder;\n";
	ret += "font-size:400%;\n";
	ret += "text-align: center;\n";
	ret += "white-space: nowrap;\n";
	ret += "}\n";
	ret += "</style>\n";
	ret += "</head>\n";
	ret += "<body><div class=\"dvd-logo\"></div></body>\n";
	ret += "<script>\n";
	ret += "let x = 0,\n";
	ret += "y = 0,\n";
	ret += "dirX = 1,\n";
	ret += "dirY = 1;\n";
	ret += "const speed = 3;\n";
	ret += "const pallete = [\"#ff8800\", \"#e124ff\", \"#6a19ff\", \"#ff2188\"];\n";
	ret += "let dvd = document.getElementById(\"dvd\");\n";
	ret += "dvd.style.color = pallete[0];\n";
	ret += "let prevColorChoiceIndex = 0;\n";
	ret += "let black = document.getElementById(\"black\");\n";
	ret += "const dvdWidth = dvd.clientWidth;\n";
	ret += "const dvdHeight = dvd.clientHeight;\n";
	ret += "function getNewRandomColor() {\n";
	ret += "const currentPallete = [...pallete]\n";
	ret += "currentPallete.splice(prevColorChoiceIndex,1)\n";
	ret += "const colorChoiceIndex = Math.floor(Math.random() * currentPallete.length);\n";
	ret += "prevColorChoiceIndex = colorChoiceIndex<prevColorChoiceIndex?colorChoiceIndex:colorChoiceIndex+1;\n";
	ret += "const colorChoice = currentPallete[colorChoiceIndex];\n";
	ret += "return colorChoice;\n";
	ret += "}\n";
	ret += "function animate() {\n";
	ret += "const screenHeight = document.body.clientHeight;\n";
	ret += "const screenWidth = document.body.clientWidth;\n";
	ret += "if (y + dvdHeight >= screenHeight || y < 0) {\n";
	ret += "dirY *= -1;\n";
	ret += "dvd.style.color = getNewRandomColor();\n";
	ret += "}\n";
	ret += "if (x + dvdWidth >= screenWidth || x < 0) {\n";
	ret += "dirX *= -1;\n";
	ret += "dvd.style.color = getNewRandomColor();\n";
	ret += "}\n";
	ret += "x += dirX * speed;\n";
	ret += "y += dirY * speed;\n";
	ret += "dvd.style.left = x + \"px\";\n";
	ret += "dvd.style.top = y + \"px\";\n";
	ret += "window.requestAnimationFrame(animate);\n";
	ret += "}\n";
	ret += "window.requestAnimationFrame(animate);\n";
	ret += "</script>\n";
	ret += "</body>\n";
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