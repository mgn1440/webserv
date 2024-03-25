#include <fstream>
#include <sstream>

#include "Location.hpp"
// limit_except GET POST;
// 		# root /path/to/root;
// 		# index index.html;
// 		# autoindex on;

// 		# fastcgi_pass unix:/var/run/php/php7.4-fpm.sock;
// 		# fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
// 		# include fastcgi_params;

Location::Location(std::ifstream& confFile)
{
	parse(confFile);
}

Location& Location::operator=(const Location& rhs)
{
	// AConfParser::operator=(rhs);
	(void)rhs;
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
			break;
		// buf => add White Space Function ??
		// ss << line;
		if (!(ss >> word))
			continue;
		else if (word == "}")
			parseClosedBracket();
		else if (word == "root")
			parseRoot(ss, word);
		else if (word == "index")
			parseIndex(ss, word);
		else if (word == "autoindex")
			parseAutoIndex(ss, word);
	}
}
