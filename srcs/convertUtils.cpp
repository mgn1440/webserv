#include <sstream>
#include <stdexcept>

size_t convertNum(std::string num)
{
	std::stringstream ss(num);
	size_t ret;
	ss >> ret;
	std::string remain;
	std::getline(ss, remain);
	if (!remain.empty() && remain.find_first_not_of(" \t\n\v\f\r") != std::string::npos)
		throw std::runtime_error("convert fail");
	return (ret);
}

size_t convertHex(std::string num)
{
	std::stringstream ss(num);
	size_t ret;
	ss >> std::hex >> ret;
	std::string remain;
	std::getline(ss, remain);
	if (!remain.empty() && remain.find_first_not_of(" \t\n\v\f\r") != std::string::npos)
		throw std::runtime_error("convert fail");
	return (ret);
}

void percentDecoding(std::string& URI)
{
	while (true){
		size_t pos = URI.find_first_of('%');
		if (pos == std::string::npos)
			break;
		char decode = convertHex(URI.substr(pos + 1, 2));
		URI.erase(pos, 3);
		URI.insert(pos, 1, decode);
	}
}

std::string intToString(size_t num)
{
    std::stringstream ss;
    ss << num;
	return ss.str();
}

std::string headerToCGIVar(const std::string& str)
{
	std::string ret = str;
	for (size_t i = 0; i < ret.length(); i++){
		ret[i] = std::toupper(static_cast<unsigned char>(ret[i]));
		if (ret[i] == '-')
			ret[i] = '_';
	}
	return ret;
}