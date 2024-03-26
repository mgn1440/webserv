#include <fstream>
#include <sstream>

#include "Location.hpp"
#include "ParseUtils.hpp"
// limit_except GET POST;
// 		# root /path/to/root;
// 		# index index.html;
// 		# autoindex on;

// 		# fastcgi_pass unix:/var/run/php/php7.4-fpm.sock;
// 		# fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
// 		# include fastcgi_params;

Location::Location()
	: AConfParser()
{ }

Location::Location(std::ifstream& confFile)
	: AConfParser()
{
	parse(confFile);
}
Location::Location(const Location& src)
{
	AConfParser::operator=(src);
}

Location& Location::operator=(const Location& rhs)
{
	if (this == &rhs)
		return (*this);
	AConfParser::operator=(rhs);
	return (*this);
}

Location::~Location()
{ }

void Location::parse(std::ifstream& confFile)
{
	std::stringstream ss;
	std::string line;
	std::string word;

	while (std::getline(confFile, line))
	{
		if (confFile.eof())
			throw std::runtime_error("Invalid location block");
		seperateMetaChar(line, "{};");
		ss.clear();
		ss << line;
		if (!(ss >> word))
			continue;
		else if (word == "}")
			return (parseClosedBracket(ss, word));
		else if (word == "root")
			parseRoot(ss, word);
		else if (word == "index")
			parseIndex(ss, word);
		else if (word == "autoindex")
			parseAutoIndex(ss, word);
		else if (word == "limit_except")
			parseLimitExcept(ss, word);
		else if (word == "cgi")
			parseCGI(ss, word);
		else
			throw std::runtime_error("Invalid symbol or syntax");
	}
}

void	Location::PrintInfo()
{
	std::cout << "HttpMethod: ";
	printVec(mHttpMethod);
	std::cout << "Root: " << mRoot << std::endl;
	std::cout << "Index: ";
	printSet(mIndex);
	std::cout << "AutoIndex: " << mbAutoIndex << std::endl;
	std::cout << "CGI: ";
	printMap(mCGI);
}