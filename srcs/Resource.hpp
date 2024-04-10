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
	std::string Upload;
	std::set<std::string> Index;
	std::map<int, std::string> ErrorPage;
	std::map<std::string, std::string> CGIBinaryPath;
	std::string ABSPath;
};

#endif