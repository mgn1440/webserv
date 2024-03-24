#ifndef CONFIGHANDLER_HPP
# define CONFIGHANDLER_HPP

# include <string>
# include <map>
# include <set>
# include <vector>

# include "Location.hpp"
# include "AConfParser.hpp"

class ConfigHandler
{
public:
	ConfigHandler(const std::string& path);
	ConfigHandler&		operator = (const ConfigHandler& rhs);
	~ConfigHandler();
private:
	ConfigHandler(const ConfigHandler& rhs);
	void createServers(const std::string& confPath);

	std::map<int, Server*> mServerMap;
};

#endif