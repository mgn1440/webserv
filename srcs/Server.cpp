#include <iostream>
#include <fstream>
#include <sstream>
#include "Server.hpp"
#include "Location.hpp"
#include "STLUtils.hpp"

bool	isDigits(const std::string &str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}


Server::Server()
	: mChkBracket(0)
	, mbInLocation(false)
	, mMaxSize()
	, mbAutoIndex()
{}

Server::Server(std::ifstream& confFile)
	: mChkBracket(0)
	, mbInLocation(false)
	, mMaxSize()
	, mbAutoIndex()
{
	parse(confFile);
}



Server::~Server(){}

Server&	Server::operator=(const Server& rhs)
{
	if (this == &rhs)
		return (*this);
	mChkBracket = rhs.mChkBracket;
	mbInLocation = rhs.mbInLocation;
	mPort = rhs.mPort;
	mServerName = rhs.mServerName;
	mErrorPage = rhs.mErrorPage;
	for (int i = 0; i < 3; i ++)
		mMaxSize[i] = rhs.mMaxSize[i];
	mHttpMethod = rhs.mHttpMethod;
	mRoot = rhs.mRoot;
	mIndex = rhs.mIndex;
	mbAutoIndex = rhs.mbAutoIndex;
	return (*this);
}

void	Server::parse(std::ifstream& confFile)
{
	std::string word;
	std::stringstream ss;
	std::string line;

	while (std::getline(confFile, line))
	{
		if (confFile.eof())
			break;
		// buf => add White Space Function ??
		// ss << line;
		if (!(ss >> word))
			continue;				 
		if (word == "location")
			parseLocation(confFile, ss, word);
		else if (word == "}")
			parseClosedBracket();
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
		throw std::exception();	// incomplete line
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
}

void	Server::PutIn(std::map<int, Server*>& rhs)
{
	for(std::set<int>::iterator it = mPort.begin(); it != mPort.end(); it ++)
		rhs[*it] = this;
}

// Parse()
//{
//  while ()
//}
void	Server::ParseLine(std::string line)
{
	std::stringstream	ss;
	std::string			word;
	static Location		location;

	ss << line;
	if (!(ss >> word))
		return ;
	if (word == "location" || mbInLocation)
		parseLocation(ss, word);
	else if (word == "}")
		parseClosedBracket();
	else if (mbInLocation)
	{
		std::cout << "inLocation" << std::endl;
		location.ParseLine(line);
		return ;
	}
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
	throw std::exception();	// incomplete line
}


void	Server::parseLocation(std::ifstream& confFile, std::stringstream& ss, std::string& word)
{
	std::string	dir;
	Location* location = new Location();
	if (!mbInLocation && ss >> dir && (ss >> word && word == "{") && !(ss >> word))
	{
		// std::cout << dir << std::endl; //print debug
		mChkBracket++;
		location.parse(confFile);
		mLocationMap[dir] = location;
		return ;
	}
	else
		throw std::exception();
}

void	Server::parseClosedBracket()
{
	mChkBracket--;
	if (mChkBracket == 0)
	{
		mbInLocation = false;
	}
	return ;
}

void	Server::parseListen(std::stringstream& ss, std::string& word)
{
	while (ss >> word)
	{
		if (isEnd(ss, word))
			return ;
		if (!isDigits(word))
			throw std::exception();
		mPort.insert(atoi(word.c_str()));
	}
}

void	Server::parseServerName(std::stringstream& ss, std::string& word)
{
	while (ss >> word)
	{
		if (isEnd(ss, word))
			return ;
		mServerName.push_back(word);
	}
}

void	Server::parseErrorPage(std::stringstream& ss, std::string& word)
{
	std::vector<int> errNum;
	while (ss >> word && isDigits(word))
		errNum.push_back(atoi(word.c_str()));
	if (errNum.empty())
		throw std::exception();
	// TODO: make function that chk word is valid URI(character, valid...)
	for (std::vector<int>::iterator it = errNum.begin(); it != errNum.end(); it ++)
		mErrorPage[*it] = word;
	if (ss >> word && isEnd(ss, word))
		return ;
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
					throw std::exception();
				size_t	tmpOrd = word.find_first_not_of("0123456789");
				if (tmpOrd != std::string::npos)
				{
					if (tmpOrd + 1 < word.length())
						throw std::exception();
					else if (word[tmpOrd] == 'K')
						mMaxSize[i] *= 1000;
					else if (word[tmpOrd] == 'M')
						mMaxSize[i] *= 1000000;
					else if (word[tmpOrd] == 'G')
						mMaxSize[i] *= 1000000000;
					else
						throw std::exception();
				}
			}
		}
		else
			throw std::exception();
	}
}
