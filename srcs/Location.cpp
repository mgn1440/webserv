#include "Location.hpp"
// limit_except GET POST;
// 		# root /path/to/root;
// 		# index index.html;
// 		# autoindex on;

// 		# fastcgi_pass unix:/var/run/php/php7.4-fpm.sock;
// 		# fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
// 		# include fastcgi_params;
Location::Location()
{ }

Location::~Location()
{ }

void	Location::ParseLine(std::string& line)
{
	if (word == "limit_except")
	{
		while (ss >> word)
		{
			if (isEnd(ss, word))
				return ;
			mHttpMethod.push_back(word);
		}
	}
	else if (word == "root")
	{
		if (ss >> mRoot && ss >> word && isEnd(ss, word))
			return ;
	}
}
