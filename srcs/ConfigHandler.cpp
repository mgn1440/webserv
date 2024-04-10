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

const size_t* ConfigHandler::GetMaxSizes(int port, std::string serverName)
{
	std::map<std::pair<int, std::string>, Server>::iterator it = mServerMap.find(serverInfo(port, serverName));

	if (it == mServerMap.end())
		return (NULL);
	return it->second.GetMaxSize();
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

std::string ConfigHandler::GetABSPath(int port, const std::string& serverName ,const std::string& URI)
{
	serverInfo info(port, serverName);
	Server& server = mServerMap[serverInfo(port, "default")];
	if (mServerMap.find(info) != mServerMap.end())
		server = mServerMap[info];
	return (server.GetABSPath(URI));
}


std::string ConfigHandler::IsCGI(const std::string& URI)
{
	return (URI);
}

struct Resource	ConfigHandler::GetResource(int port, const std::string& serverName, const std::string& URI)
{
	serverInfo info(port, serverName);
	// std::cout << "port = " << port << '\n';
	// std::cout << "serverName = " << serverName << '\n';
	Server& server = mServerMap[serverInfo(port, "default")];
	if (mServerMap.find(info) != mServerMap.end())
		server = mServerMap[info];
	struct Resource resource = server.GetResource(URI);
	resource.ABSPath = server.GetABSPath(URI);
	return resource;
}

std::deque<Response> ConfigHandler::GetResponseOf(std::vector<struct Request> requests)
{
	std::deque<Response> responseDeq;

	for (std::vector<Request>::iterator it = requests.begin(); it != requests.end(); it ++)
	{
		// TODO:
		// request 유효성 검사(serv_name, port => configHandler)
		// 1. vaild 한 method인지 확인 => invalid 할 시 501() return 
		// 2. root를 따라가면서 URI를 absolute Path로 변환하고 실제 파일이 존재하는지 확인 => 404 return
		// 3. valid 한 HTTP version인지 확인 => invalid 할 시 error() return
		Response response;
		responseDeq.push_back(response);
		// method에 따라서 분기
		// // 1. GET
		// 	HTTP ver, status code, status 

		// 	Date 
		// 	Server: WebServ
		// 	Content-Length: 
		// 	Content-type:

		// 	Body: 
		// // 2. HEAD
		// 	HTTP ver, status code, status 

		// 	Date 
		// 	Server: WebServ
		// 	Content-Length: 
		// 	Content-type:
		// // 3. POST almost everything is cgi
		// 	HTTP ver, status code, status 

		// 	Date 
		// 	Server: WebServ
		// 	Content-Length: 
		// 	Content-type:
		// // 4. PUT
		// // 5. DELETE
		// 	HTTP ver, status code, status 

		// 	Date 
		// 	Server: WebServ

		// 	만약 
		// 	Content-Length: 
		// 	Content-type:

		// 	Body:
		// Response response = Response();
		// responseDeq.push_back(response);
	}
	return responseDeq;
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

// for DEBUG
void ConfigHandler::PrintAll()
{
	std::cout << "~~ typesInfo ~~" << std::endl;
	// for (std::map<std::string, std::string>::iterator it = mTypeMap.begin(); it != mTypeMap.end(); it ++)
	// 	std::cout << it->first << " : " << it->second << std::endl;
	printMap(mTypeMap);
	std::cout << "~~ serverInfo ~~" << std::endl;
	for (std::map<serverInfo, Server>::iterator it = mServerMap.begin(); it != mServerMap.end(); it ++)
	{
		it->second.PrintInfo();
		std::cout << std::endl;
	}
	printSet(mPortSet);
}

void ConfigHandler::PrintServInfo(serverInfo info)
{
	std::map<serverInfo, Server>::iterator it = mServerMap.find(info);
	if (it == mServerMap.end())
	{
		std::cout << "cannot find port" << std::endl;
		return ;
	}
	it->second.PrintInfo();
}

