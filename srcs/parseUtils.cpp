#include <string>
#include <vector>

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
