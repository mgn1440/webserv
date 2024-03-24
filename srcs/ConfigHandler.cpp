#include <fstream>
#include "ConfigHandler.hpp"
#include "Server.hpp"

void ConfigHandler::createServers(const std::string& confPath)
{
	std::ifstream confFile(confPath);

	if (!confFile.is_open())
		throw std::runtime_error("can't confile open");
	while(true)
	{
		if (confFile.eof())
			break;
		Server *server = new Server(confFile);
		server->PutIn(mServerMap);
	} // doing this
}