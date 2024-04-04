#include <string>
#include <vector>

void trim(std::string& str, const std::string& del)
{
	str.erase((str.find_last_not_of(del)) + 1);
	str.erase(0, str.find_first_not_of(del));
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

void refreshBuffer(std::string& buf, size_t& size)
{
	buf.erase(0, size);
	size = 0;
}
