#include <fstream>
#include "ConfigHandler.hpp"
#include "Server.hpp"

void ConfigHandler::parseConfig(const std::string& confPath)
{
	std::ifstream confFile(confPath);

	if (!confFile.is_open())
		throw std::runtime_error("can't open conf file");

	std::string line;
	std::string word;
	std::stringstream ss;

	while(std::getline(confFile, line))
	{
		if (confFile.eof())
			break;
		// line => Add White Space ?? ;
		// ss << line ;
		// server 
		if (!(ss >> word))
			continue;
		else if (word == "server")
			createServer(ss, confFile);
		else if (word == "}")
			parseClosedBracket();
		else 
			throw std::domain_error("Config File Syntax Error");
	}
}


void	ConfigHandler::createServer(std::stringstream& ss, std::ifstream& confFile)
{
	std::string word;

	if ((ss >> word && word == "{") && !(ss >> word))
	{
		mbInBracket = true;
		Server server(confFile);
		server.PutIn(mServerMap);
	}
	else
		throw std::domain_error("Server Open Bracket Error");
}

void	ConfigHandler::parseClosedBracket(void)
{
	if (mbInBracket)
		mbInBracket = false;
	else
		throw std::domain_error("Server Close Bracket Error");
}