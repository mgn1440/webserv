#include <iostream>
#include <fstream>
#include <sstream>
#include "Server.hpp"
#include "STLUtils.hpp"
#include "parseUtils.hpp"

Server::Server()
	: AConfParser()
{ }

Server::Server(std::ifstream& confFile)
	: AConfParser()
	, mbIsDuplicatedClientMaxSize(false)
{
	mMaxSize[0] = DEF_ST_LINE_SIZE;
	mMaxSize[1] = DEF_HEADER_SIZE;
	mMaxSize[2] = DEF_BODY_SIZE;
	parse(confFile);
	if (mPort.empty())
		throw std::runtime_error("Does not define port");
	if (mHttpMethod.empty())
	{
		mHttpMethod.push_back("GET");
		mHttpMethod.push_back("POST");
		mHttpMethod.push_back("DELETE");
	}
}

Server::~Server()
{ }

Server&	Server::operator=(const Server& rhs)
{
	if (this == &rhs)
		return (*this);
	AConfParser::operator=(rhs);
	mPort = rhs.mPort;
	mbIsDuplicatedClientMaxSize = rhs.mbIsDuplicatedClientMaxSize;
	mServerName = rhs.mServerName;
	mErrorPage = rhs.mErrorPage;
	for (int i = 0; i < 3; i ++)
		mMaxSize[i] = rhs.mMaxSize[i];
	mLocationMap = rhs.mLocationMap;
	return (*this);
}

void	Server::parse(std::ifstream& confFile)
{
	std::stringstream ss;
	std::string line;
	std::string word;

	while (std::getline(confFile, line))
	{
		seperateMetaChar(line, "{};");
		ss.clear();
		ss << line;
		if (!(ss >> word))
			continue;
		if (word == "location")
			parseLocation(confFile, ss, word);
		else if (word == "}")
			return (parseClosedBracket(ss, word));
		else if (word == "listen")
			parseListen(ss, word);
		else if (word == "server_name")
			parseServerName(ss, word);
		else if (word == "error_page")
			parseErrorPage(ss, word);
		else if (word == "client_max_size")
			parseClientMaxSize(ss, word);
		else if (word == "root")
			parseRoot(ss, word);
		else if (word == "autoindex")
			parseAutoIndex(ss, word);
		else if (word == "limit_except")
			parseLimitExcept(ss, word);
		else if (word == "index")
			parseIndex(ss, word); // TODO: why couldn't saved?
		else if (word == "cgi")
			parseCGI(ss, word);
		else
			throw std::runtime_error("Invalid symbol or syntax, may be duplicated");
		if (confFile.eof())
			throw std::runtime_error("invalid server block");
	}
}


void	Server::PrintInfo()
{
	std::cout << "Port: ";
	printSet(mPort);
	std::cout << "ErrorPage: ";
	printMap(mErrorPage);
	std::cout << "MaxSize: ";
	for (int i = 0; i < 3; i ++)
		std::cout << mMaxSize[i] << " ";
	std::cout << std::endl;
	std::cout << "ServerName: ";
	printVec(mServerName);
	std::cout << "HttpMethod: ";
	printVec(mHttpMethod);
	std::cout << "Root: " << mRoot << std::endl;
	std::cout << "Index: ";
	if (mIndex.empty())
		std::cout << "INDEX is empty!!" << std::endl;
	printSet(mIndex);
	std::cout << "AutoIndex: " << mbAutoIndex << std::endl;
	std::cout << "CGI: ";
	printMap(mCGI);
	for (std::map<std::string, Location>::iterator it = mLocationMap.begin(); it != mLocationMap.end(); it++){
		std::cout << "location ";
		std::cout << it->first;
		std::cout << " {" << std::endl;
		it->second.PrintInfo();
		std::cout << "}" << std::endl;
	}
}

void	Server::PutIn(std::map<serverInfo, Server>& rhs)
{
	for(std::set<int>::iterator portIt = mPort.begin(); portIt != mPort.end(); portIt ++)
	{
		serverInfo info = serverInfo(*portIt, "default");
		if(rhs.find(info) == rhs.end())
			rhs[info] = *this;
		for(std::vector<std::string>::iterator servNameIt = mServerName.begin(); servNameIt != mServerName.end(); servNameIt ++)
		{
			info = serverInfo(*portIt, *servNameIt);
			if (rhs.find(info) == rhs.end())
				rhs[info] = *this;
			else
				throw std::runtime_error("port duplicated");
		}
	}
}

const size_t* Server::GetMaxSize()
{
	return mMaxSize;
}

std::set<int>& Server::GetPorts()
{
	return mPort;
}

struct Resource Server::GetResource(std::string URI)
{
	struct Resource res;

	res.BAutoIndex = mbAutoIndex;
	res.CGI = mCGI;
	res.ErrorPage = mErrorPage;
	res.HttpMethod = mHttpMethod;
	res.Index = mIndex;
	res.Root = mRoot;
	std::string locationPath = searchLocationPath(URI);
	if (locationPath != "")
		mLocationMap[locationPath].SetResource(res);
	return (res);
}


std::string Server::searchLocationPath(const std::string& URI)
{
	std::string locationPath = "";

	for (std::map<std::string, Location>::iterator it = mLocationMap.begin(); it != mLocationMap.end(); it ++)
	{
		if (URI.find(it->first) == 0 && locationPath.length() < it->first.length())
			locationPath = it->first;
	}
	return (locationPath);
}

// TODO: request에서 URI의 맨 마지막에 slash 다 제거
std::string Server::GetABSPath(const std::string& URI)
{
	std::string path = mRoot;
	std::string locationPath = searchLocationPath(URI);

	if (locationPath != "")
		mLocationMap[locationPath].GetRoot(path);
	return path + URI;
}

void	Server::parseLocation(std::ifstream& confFile, std::stringstream& ss, std::string& word)
{
	std::string	dir;
	if (ss >> dir && (ss >> word && word == "{") && !(ss >> word))
	{
		Location location(confFile);
		if (mLocationMap.find(dir) == mLocationMap.end())
			mLocationMap[dir] = location;
		else
			throw std::runtime_error("Duplication of location");
	}
	else
		throw std::runtime_error("Location Open Bracket Error");
}

void	Server::parseListen(std::stringstream& ss, std::string& word)
{
	int	port;
	if (!mPort.empty())
		throw std::runtime_error("listen syntax duplicated");
	while (ss >> word)
	{
		if (isEnd(ss, word))
		{
			if (mPort.empty())
				break;
			return ;
		}
		if (!isDigits(word))
			break;
		port = atoi(word.c_str());
		mPort.insert(port);
	}
	throw std::runtime_error("Wrong listen format");
}

void	Server::parseServerName(std::stringstream& ss, std::string& word)
{
	if (!mServerName.empty())
		throw std::runtime_error("server name syntax duplicated");
	while (ss >> word)
	{
		if (isEnd(ss, word))
		{
			if (mServerName.empty())
				break;
			return ;
		}
		mServerName.push_back(word);
	}
	throw std::runtime_error("Wrong server name format");
}

void	Server::parseErrorPage(std::stringstream& ss, std::string& word)
{
	std::vector<int> errNum;
	while (ss >> word && isDigits(word))
		errNum.push_back(atoi(word.c_str()));
	if (errNum.empty())
		throw std::runtime_error("Wrong error page format");
	// TODO: make function that chk word is valid URI(character, valid...)
	for (std::vector<int>::iterator it = errNum.begin(); it != errNum.end(); it ++)
	{
		if (mErrorPage.find(*it) == mErrorPage.end())
			mErrorPage[*it] = word;
		else
			throw std::runtime_error("Wrong error page format");	
	}
	if (ss >> word && isEnd(ss, word))
		return ;
	throw std::runtime_error("Wrong error page format");
}

void	Server::parseClientMaxSize(std::stringstream& ss, std::string& word)
{
	if (mbIsDuplicatedClientMaxSize == true)
		throw std::runtime_error("client max size duplicated");
	mbIsDuplicatedClientMaxSize = true;
	for (int i = 0; i < 4; i ++)
	{
		if (ss >> word)
		{
			if (word == ";" && isEnd(ss, word) && i == 3)
				return ;
			else
			{
				mMaxSize[i] = atoi(word.c_str());
				if (mMaxSize[i] <= 0)
					throw std::runtime_error("wrong client max size format");
				size_t	tmpOrd = word.find_first_not_of("0123456789");
				if (tmpOrd != std::string::npos)
				{
					if (tmpOrd + 1 < word.length())
					throw std::runtime_error("wrong client max size format");
					else if (word[tmpOrd] == 'K')
						mMaxSize[i] *= 1000;
					else if (word[tmpOrd] == 'M')
						mMaxSize[i] *= 1000000;
					else if (word[tmpOrd] == 'G')
						mMaxSize[i] *= 1000000000;
					else
						throw std::runtime_error("wrong client max size format");
				}
			}
		}
		else
			throw std::runtime_error("wrong client max size format");
	}
}
