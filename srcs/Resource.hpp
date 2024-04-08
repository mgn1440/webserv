#ifndef RESOURCE_HPP
# define RESOURCE_HPP

# include <vector>
# include <string>
# include <set>
# include <map>
struct Resource
{
	bool BAutoIndex;
	std::vector<std::string> HttpMethod;
	std::string Root;
	std::set<std::string> Index;
	std::map<std::string, std::string> CGI;
	std::map<int, std::string> ErrorPage;
	std::string ABSPath;
};

#endif