#include <iostream>
#include <fstream>
#include <sstream>
#include "Server.hpp"
#include "STLUtils.hpp"
#include "parseUtils.hpp"

Server::Server()
	: AConfParser()
{}

Server::Server(std::ifstream& confFile)
	: AConfParser()
	, mbIsDuplicatedUpload(false)
	, mUpload("/upload")
{
	parse(confFile);
	if (mPort.empty())
		throw std::runtime_error("Does not define port");
	if (mHttpMethod.empty())
	{
		mHttpMethod.push_back("GET");
		mHttpMethod.push_back("POST");
		mHttpMethod.push_back("DELETE");
	}
	mUpload = mRoot + mUpload;
}

Server::~Server()
{}

Server&	Server::operator=(const Server& rhs)
{
	if (this == &rhs)
		return (*this);
	AConfParser::operator=(rhs);
	mPort = rhs.mPort;
	mServerName = rhs.mServerName;
	mErrorPage = rhs.mErrorPage;
	mLocationMap = rhs.mLocationMap;
	return (*this);
}

void Server::parse(std::ifstream& confFile)
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
			parseIndex(ss, word);
		else if (word == "cgi")
			parseCGI(ss, word);
		else if (word == "upload")
			parseUpload(ss, word);
		else if (word == "return")
			parseReturn(ss, word);
		else
			throw std::runtime_error("Invalid symbol or syntax, may be duplicated");
		if (confFile.eof())
			throw std::runtime_error("invalid server block");
	}
}


void Server::PrintInfo()
{
	std::cout << "Port: ";
	printSet(mPort);
	std::cout << "ErrorPage: ";
	printMap(mErrorPage);
	std::cout << "MaxSize: ";
	std::cout << mMaxSize << " ";
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
	for (std::map<std::string, Location>::iterator it = mLocationMap.begin(); it != mLocationMap.end(); it++)
	{
		std::cout << "location ";
		std::cout << it->first;
		std::cout << " {" << std::endl;
		it->second.PrintInfo();
		std::cout << "}" << std::endl;
	}
}

void Server::PutIn(std::map<int, Server>& rhs)
{
	for(std::set<int>::iterator portIt = mPort.begin(); portIt != mPort.end(); portIt ++)
	{
		if(rhs.find(*portIt) != rhs.end())
			throw std::runtime_error("port duplicated");
		rhs[*portIt] = *this;
	}
}

size_t Server::GetMaxSize(std::string& URI)
{
	size_t res = mMaxSize;

	std::string locationPath = searchLocationPath(URI);
	if (locationPath != "")
		mLocationMap[locationPath].GetMaxSize(res);
	return res;
}

std::set<int>& Server::GetPorts()
{
	return mPort;
}

struct Resource Server::GetResource(std::string URI)
{
	struct Resource res;

	res.BAutoIndex = mbAutoIndex;
	res.CGIBinaryPath = mCGI;
	res.HttpMethod = mHttpMethod;
	res.Index = mIndex;
	res.Root = mRoot;
	res.RedirCode = mRedirCode;
	res.Location = mLocation;
	res.ErrorPage = mErrorPage;
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

std::string Server::GetABSPath(const std::string& URI)
{
	std::string path = mRoot;
	std::string locationPath = searchLocationPath(URI);

	if (locationPath != "")
		mLocationMap[locationPath].GetRoot(path);
	if (URI == locationPath)
		return path;
	path += URI.substr(locationPath.size());
	if (path[path.size() - 1] == '/')
		path.erase(path.size() - 1);
	return path;
}

void Server::parseLocation(std::ifstream& confFile, std::stringstream& ss, std::string& word)
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

void Server::parseListen(std::stringstream& ss, std::string& word)
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
		if (mPort.find(port) != mPort.end())
			break;
		mPort.insert(port);
	}
	throw std::runtime_error("Wrong listen format");
}

void Server::parseServerName(std::stringstream& ss, std::string& word)
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

void Server::parseErrorPage(std::stringstream& ss, std::string& word)
{
	std::vector<int> errNum;
	while (ss >> word && isDigits(word))
		errNum.push_back(atoi(word.c_str()));
	if (errNum.empty())
		throw std::runtime_error("Wrong error page format");
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


void Server::parseUpload(std::stringstream& ss, std::string& word)
{
	if (mbIsDuplicatedUpload == true)
		throw std::runtime_error("upload duplicated");
	mbIsDuplicatedUpload = true;
	if (ss >> mUpload && ss >> word && isEnd(ss, word))
	{
		if (mUpload.find("//") != std::string::npos)
			throw std::runtime_error("invalid upload path");
		if (mUpload[mUpload.size()-1] == '/' && mUpload.length() > 1)
			mUpload.erase(mUpload.size()-1);
		return ;
	}
	throw std::runtime_error("bad end logic");
}
