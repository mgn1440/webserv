#include <iostream>
#include <fstream>

#include "ConfigHandler.hpp"
#include "Server.hpp"
#include "ParseUtils.hpp"

void	exitWithError(std::string msg)
{
	std::cerr << "Error: " << msg << std::endl;
	exit(1);
}

ConfigHandler::ConfigHandler(const std::string& confPath)
{
	parseConfig(confPath);
}

ConfigHandler::~ConfigHandler()
{ }


void ConfigHandler::PrintInfo(int port)
{
	std::map<int, Server>::iterator it = mServerMap.find(port);
	if (it == mServerMap.end())
	{
		std::cout << "Cannot find port" << std::endl;
		return ;
	}
	it->second.PrintInfo();
}

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
		seperateMetaChar(line, "{};");
		ss.clear();
		ss << line;
		if (!(ss >> word))
			continue;
		else if (word == "server")
			createServer(ss, confFile);
		else
		{
			std::cout << line << std::endl;
			throw std::runtime_error("Config File Syntax Error");
		}
	}
}


void	ConfigHandler::createServer(std::stringstream& ss, std::ifstream& confFile)
{
	std::string word;

	if ((ss >> word && word == "{") && !(ss >> word))
	{
		Server server(confFile);
		server.PutIn(mServerMap);
	}
	else
		throw std::runtime_error("Server Open Bracket Error");
}

void	ConfigHandler::parseClosedBracket(std::stringstream& ss, std::string& word)
{
	if (ss >> word)
		throw std::runtime_error("Wrong bracket form");
	return ;
}

const long long* ConfigHandler::GetMaxSizes(int port)
{
	std::map<int, Server>::iterator it = mServerMap.find(port);

	if (it == mServerMap.end())
		return (NULL);
	return it->second.GetMaxSize();
} 