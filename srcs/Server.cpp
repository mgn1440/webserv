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

bool	isEnd(std::stringstream& ss, std::string& word)
{
	if (word != ";")
		return (false);
	else if (ss >> word)
		throw std::exception();
	else
		return (true);
}

Server::Server() : mChkBracket(0), mbInLocation(false)
{ }

Server::~Server() { }

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

void	Server::PutIn(std::map<int, Server>& rhs)
{
	for(std::set<int>::iterator it = mPort.begin(); it != mPort.end(); it ++)
		rhs[*it] = *this;
}

void	Server::ParseLine(std::string line)
{
	std::stringstream	ss;
	std::string			word;
	static Location		location;

	ss << line;
	if (!(ss >> word))
		return ;
	if (word == "location")
	{
		std::string	dir;
		if (!mbInLocation && ss >> dir && (ss >> word && word == "{") && !(ss >> word))
		{
			std::cout << dir << std::endl;
			mbInLocation = true;
			mChkBracket ++;
			return ;
		}
		else
			throw std::exception();
	}
	else if (word == "}")
	{
		mChkBracket --;
		if (mChkBracket == 0)
		{
			mbInLocation = false;
		}
		return ;
	}
	else if (mbInLocation)
	{
		std::cout << "inLocation" << std::endl;
		location.ParseLine(line);
		return ;
	}
	else if (word == "listen")
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
	else if (word == "server_name")
	{
		while (ss >> word)
		{
			if (isEnd(ss, word))
				return ;
			mServerName.push_back(word);
		}
	}
	else if (word == "error_page")
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
	else if (word == "client_max_size")
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
	else if (word == "root")
	{
		if (ss >> mRoot && ss >> word && isEnd(ss, word))
			return ;
	}
	else if (word == "autoindex")
	{
		if (ss >> word)
		{
			if (word == "on")
				mbAutoIndex = true;
			else if (word == "off")
				mbAutoIndex = false;
			else
				throw std::exception();
			if (ss >> word && isEnd(ss, word))
				return ;
		}
		else
			throw std::exception();
	}
	else if (word == "limit_except")
	{
		while (ss >> word)
		{
			if (isEnd(ss, word))
				return ;
			mHttpMethod.push_back(word);
		}
	}
	else if (word == "index")
	{
		while (ss >> word)
		{
			if (isEnd(ss, word))
				return ;
			mIndex.insert(word);
		}
	}
	throw std::exception();	// incomplete line
}