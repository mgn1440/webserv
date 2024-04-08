#include <fstream>
#include <sstream>

#include "Location.hpp"
#include "parseUtils.hpp"
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
	mRoot = "";
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

void Location::GetRoot(std::string& path)
{
	if (mRoot != "")
		path = mRoot;
}

// server {
// 	root /var;
// 	index index.html;
// 	cgi php /usr/bin/php7.1;
// 	location /html {
// 		root /var/www;
// 		index index.php;
// 		cgi php /usr/bin/php8.1;
// 	}
// }
// URI(입력): /html/index.php => ABSPath(결과): /var/www/html/index.php(8.1)

void Location::SetResource(struct Resource& res)
{
	res.HttpMethod = mHttpMethod;
	if (mRoot != "")
		res.Root = mRoot;
	if (!mIndex.empty())
		res.Index = mIndex;
	for (std::map<std::string, std::string>::iterator it = mCGI.begin(); it != mCGI.end(); it ++)
	{
		res.CGI[it->first] = it->second;
	}
}

void Location::parse(std::ifstream& confFile)
{
	std::stringstream ss;
	std::string line;
	std::string word;

	while (std::getline(confFile, line))
	{
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
		if (confFile.eof())
			throw std::runtime_error("Invalid location block");
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
