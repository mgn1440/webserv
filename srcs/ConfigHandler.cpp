#include <iostream>
#include <fstream>
#include <sstream>
#include <deque>
#include "ConfigHandler.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Resource.hpp"
#include "parseUtils.hpp"

ConfigHandler* ConfigHandler::configHandler = NULL;

ConfigHandler::ConfigHandler(const std::string& confPath)
{
    parseConfig(confPath);
}

void ConfigHandler::MakeConfigHandler(const std::string& confPath)
{
    if (configHandler != NULL)
        throw std::logic_error("already initiallized config file");
    else
        configHandler = new ConfigHandler(confPath);
}

ConfigHandler& ConfigHandler::GetConfigHandler()
{
    if (configHandler != NULL)
        return (*configHandler);
    throw std::logic_error("doesn't initiallized config file");
}

size_t ConfigHandler::GetMaxSize(int port, std::string& URI)
{
	return (mServerMap[port].GetMaxSize(URI));
}

std::set<int>& ConfigHandler::GetPorts()
{
	return mPortSet;
}

std::string ConfigHandler::GetContentType(const std::string& URI)
{
	std::string type = URI.substr(URI.find_last_of(".") + 1);
	if (mTypeMap.find(type) != mTypeMap.end())
		return (mTypeMap[type]);
	return ("text/plain");
}

std::string ConfigHandler::GetABSPath(int port, const std::string& URI)
{
	Server& server = mServerMap[port];
	return (server.GetABSPath(URI));
}

struct Resource	ConfigHandler::GetResource(int port, const std::string& URI)
{
	Server& server = mServerMap[port];
	struct Resource resource = server.GetResource(URI);
	resource.ABSPath = server.GetABSPath(URI);
	resource.URI = URI;
	return resource;
}

void ConfigHandler::parseConfig(const std::string& confPath)
{
	std::ifstream confFile(confPath);

	if (!confFile.is_open())
		throw std::runtime_error("can't open config file");

	std::string line;
	std::string word;
	std::stringstream ss;

	while(std::getline(confFile, line))
	{
		seperateMetaChar(line, "{};");
		ss.clear();
		ss << line;
		if (!(ss >> word))
			continue;
        else if (word == "types")
            createTypes(ss, confFile);
		else if (word == "server")
			createServer(ss, confFile);
		else
		{
			std::cout << line << std::endl;
			throw std::runtime_error("config file syntax error");
		}
		if (confFile.eof())
			break;
	}
	if (mServerMap.empty())
		throw std::runtime_error("no server in config file");
}

bool ConfigHandler::isEnd(std::stringstream& ss, std::string& word)
{
	if (word != ";")
		return (false);
	else if (ss >> word)
		throw std::exception();
	else
		return (true);
}

void ConfigHandler::parseClosedBracket(std::stringstream& ss, std::string& word)
{
	if (ss >> word)
		throw std::runtime_error("wrong bracket format");
}

void ConfigHandler::chkValidFormOf(std::string& type, std::string& form)
{
	size_t idx = type.find("/");
	std::string subType = type.substr(idx + 1);

	if (idx == 0 || idx == std::string::npos || type.find("/", idx + 1) != std::string::npos || subType == "" || !isAlnums(form))
		throw std::runtime_error("invalid type form");
}


void ConfigHandler::parseTypeLine(std::stringstream& ss, std::string& word)
{
    std::string form;
    int idx = 0;

    while (ss >> form)
    {
        if (isEnd(ss, form) && idx > 0)
            return ;
		idx ++;
		// TODO: chk the form of format(alphabet and digits)
		chkValidFormOf(word, form);
        if (mTypeMap.find(form) == mTypeMap.end())
            mTypeMap[form] = word;
        else
            throw std::runtime_error("type form duplicated");
    }
    throw std::runtime_error("invalid type line");
}

void ConfigHandler::createTypes(std::stringstream& ss, std::ifstream& confFile)
{
	std::string word, form;
    std::string line;

	if (!(ss >> word && word == "{") || (ss >> word))
		throw std::runtime_error("invalid types bracket");
    while (std::getline(confFile, line))
    {
		seperateMetaChar(line, "{};");
		ss.clear();
		ss << line;
		if (!(ss >> word))
			continue;
		if (word == "}")
            return parseClosedBracket(ss, word);
        else
            parseTypeLine(ss, word);
        if (confFile.eof())
			throw std::runtime_error("invalid types block");
    }
}

void ConfigHandler::createServer(std::stringstream& ss, std::ifstream& confFile)
{
	std::string word;

	if ((ss >> word && word == "{") && !(ss >> word))
	{
		Server server(confFile);
		server.PutIn(mServerMap);
		std::set<int>& ports = server.GetPorts();
		for (std::set<int>::iterator it = ports.begin(); it != ports.end(); it ++)
			mPortSet.insert(*it);
	}
	else
		throw std::runtime_error("invalid server bracket");
}

void ConfigHandler::PrintServInfo(int port)
{
	std::map<int, Server>::iterator it = mServerMap.find(port);
	if (it == mServerMap.end())
	{
		std::cout << "cannot find port" << std::endl;
		return ;
	}
	it->second.PrintInfo();
}
