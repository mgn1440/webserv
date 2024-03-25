#ifndef CONFIG_HANDLER_HPP
# define CONFIG_HANDLER_HPP

# include <string>
# include <map>
# include <set>
# include <vector>

# include "Server.hpp"

class ConfigHandler
{
public:
	ConfigHandler(const std::string& confPath);
	ConfigHandler& operator=(const ConfigHandler& rhs);
	~ConfigHandler();

	void PrintInfo(int port);
	int* getMaxSizes();
private:
	ConfigHandler(const ConfigHandler& rhs);

	void parseConfig(const std::string& confPath);
	void createServer(std::stringstream& ss, std::ifstream& confFile);
	void parseClosedBracket(std::stringstream& ss, std::string& word);

	std::map<int, Server> mServerMap; 
	bool mbInBracket;
};

#endif