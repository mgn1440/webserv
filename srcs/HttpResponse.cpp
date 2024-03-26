//  Create HttpResponse class
//  Parse nginx configuration file while constructing class
//  Create General nginx configuration file
//  Save parsed information on server and location maps

# include <string>
# include <fstream>
# include <sstream>

#include "HttpResponse.hpp"

HttpResponse::HttpResponse(std::string confFile)
{
	parseNginxConfig(confFile);
}

HttpResponse::~HttpResponse() { }

void	HttpResponse::parseNginxConfig(std::string confFile)
{
	std::ifstream		ifs(confFile);
	std::stringstream	ss;
	
	ss << ifs;
	std::string			t1;

	ss >> t1;
	while (t1 != "")
	{
		std::cout << t1 << " " << std::endl;

	}
}