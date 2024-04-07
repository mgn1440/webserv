#include <iostream>

#include "parseUtils.hpp"

std::string	seperateMetaChar(std::string& line, std::string set)
{
	size_t	i = 0;
	size_t	len = 0;
	line = line.substr(0, (int)line.find('#'));
	for (char c = set[i]; i < set.length(); i ++, c = set[i])
	{
		size_t	j = line.find(c);
		while (j != std::string::npos)
		{
			len ++;
			line.replace(j, 1, " " + std::string(1, c) + " ");
			j = line.find(c, j + 2);
		}
	}
	if (len > 1)
		exitWithError("to many meta character");
	return line;
}

void	exitWithError(const std::string& msg)
{
	std::cerr << "Error: " << msg << std::endl;
	exit(1);
}

bool isWhitespaces(const std::string &str)
{
    return str.find_first_not_of("\r\v\n\t\f") == std::string::npos;
}

std::string	seperateMetaChar(std::string line, std::string set)
{
	size_t	i = 0;
	size_t	len = 0;
	for (char c = set[i]; i < set.length(); i ++, c = set[i])
	{
		size_t	j = line.find(c);
		while (j != std::string::npos)
		{
			len ++;
			line.replace(j, 1, " " + std::string(1, c) + " ");
			j = line.find(c, j + 2);
		}
	}
	if (len > 1)
		exitWithError(" to many meta character");
	return line;
}

std::string trim(std::string& str)
{
    std::string ret = str.erase((str.find_last_not_of(" \t\n\v\f\r")) + 1);
    return (ret.erase(0, ret.find_first_not_of(" \t\n\v\f\r")));
}

std::vector<std::string> split(const std::string& str, std::string delim)
{
    int pre = 0;
    int cur = 0;
    std::vector<std::string> ret;

    std::string token;
    while (static_cast<const unsigned long>(cur) != std::string::npos)
    {
        cur = str.find(delim, pre);
        token = str.substr(pre, cur - pre);
        ret.push_back(token);
        pre = cur + 1;
    }
    ret.push_back(token);
    return ret;
}

bool checkCRLF(std::string& str)
{
    return (str.back() == '\r');
}

void refreshBuffer(std::string& buf, int& size)
{
    buf.erase(0, size);
    size = 0;
}

bool isDigits(const std::string& str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}

bool isAlphas(const std::string& str)
{
	return str.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") == std::string::npos;
}

bool isAlnums(const std::string& str)
{
	return str.find_first_not_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ") == std::string::npos;
}