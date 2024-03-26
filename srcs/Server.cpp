#include "Server.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include "STLUtils.hpp"
#include "ParseUtils.hpp"

bool	isDigits(const std::string &str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
} 


Server::Server()
	: AConfParser()
{}

Server::Server(std::ifstream& confFile)
	: AConfParser()
{
	parse(confFile);
}

Server::~Server()
{ }

Server&	Server::operator=(const Server& rhs)
{
	if (this == &rhs)
		return (*this);
	mPort = rhs.mPort;
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
		if (confFile.eof())
			throw std::runtime_error("Invalid server block");
		chkAndSeperateMetaChar(line, "{};");
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
			parseIndex(ss, word);
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
	printSet(mIndex);
	std::cout << "AutoIndex: " << mbAutoIndex << std::endl;
	for (std::map<std::string, Location>::iterator it = mLocationMap.begin(); it != mLocationMap.end(); it++){
		std::cout << "location ";
		std::cout << it->first;
		std::cout << " {" << std::endl;
		it->second.PrintInfo();
		std::cout << "}" << std::endl;
	}
}

void	Server::PutIn(std::map<int, Server>& rhs)
{
	for(std::set<int>::iterator it = mPort.begin(); it != mPort.end(); it ++)
		rhs[*it] = *this;
}

const long long* Server::GetMaxSize()
{
	return mMaxSize;
}

void	Server::parseLocation(std::ifstream& confFile, std::stringstream& ss, std::string& word)
{
	std::string	dir;
	if (ss >> dir && (ss >> word && word == "{") && !(ss >> word))
	{
		// std::cout << dir << std::endl; //print debug
		Location location(confFile);
		mLocationMap[dir] = location;
	}
	else
		throw std::exception();
}

void	Server::parseListen(std::stringstream& ss, std::string& word)
{
	while (ss >> word)
	{
		if (isEnd(ss, word))
			return ;
		if (!isDigits(word))
			break;
		mPort.insert(atoi(word.c_str()));
	}
	throw std::runtime_error("Wrong listen format");
}

void	Server::parseServerName(std::stringstream& ss, std::string& word)
{
	while (ss >> word)
	{
		if (isEnd(ss, word))
			return ;
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
		mErrorPage[*it] = word;
	if (ss >> word && isEnd(ss, word))
		return ;
	throw std::runtime_error("Wrong error page format");
}

void	Server::parseClientMaxSize(std::stringstream& ss, std::string& word)
{
	for (int i = 0; i < 4; i ++)
	{
		if (ss >> word)
		{
			if (word == "inf")
				mMaxSize[i] = -1;
			else if (word == ";" && isEnd(ss, word) && i == 3)
				return ;
			else
			{
				mMaxSize[i] = atoi(word.c_str());
				if (mMaxSize[i] <= 0)
					throw std::runtime_error("Wrong client max size format");
				size_t	tmpOrd = word.find_first_not_of("0123456789");
				if (tmpOrd != std::string::npos)
				{
					if (tmpOrd + 1 < word.length())
					throw std::runtime_error("Wrong client max size format");
					else if (word[tmpOrd] == 'K')
						mMaxSize[i] *= 1000;
					else if (word[tmpOrd] == 'M')
						mMaxSize[i] *= 1000000;
					else if (word[tmpOrd] == 'G')
						mMaxSize[i] *= 1000000000;
					else
						throw std::runtime_error("Wrong client max size format");
				}
			}
		}
		else
			throw std::runtime_error("Wrong client max size format");
	}
}
