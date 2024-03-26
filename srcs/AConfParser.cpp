#include "AConfParser.hpp"

AConfParser::AConfParser()
	: mbAutoIndex(false)
	, mRoot("/")
{}

AConfParser::AConfParser(const AConfParser& rhs)
{
	static_cast<void>(rhs);
}

AConfParser::~AConfParser()
{}

AConfParser& AConfParser::operator=(const AConfParser& rhs)
{
	if (this == &rhs)
		return *this;
	mbAutoIndex = rhs.mbAutoIndex;
	mHttpMethod = rhs.mHttpMethod;
	mRoot = rhs.mRoot;
	mIndex = rhs.mIndex;
	mCgi = rhs.mCgi;
	return *this;
}

void	AConfParser::parseRoot(std::stringstream& ss, std::string& word)
{
	if (ss >> mRoot && ss >> word && isEnd(ss, word))
		return ;
}

void	AConfParser::parseIndex(std::stringstream& ss, std::string& word)
{
	while (ss >> word)
	{
		if (isEnd(ss, word))
			return ;
		mIndex.insert(word);
	}
}

void	AConfParser::parseCgi(std::stringstream& ss, std::string& word)
{
	std::string cgiExec;	
	if ((ss >> word) && (ss >> cgiExec))
	{
		mCgi[word] = cgiExec;
		if (ss >> word && isEnd(ss, word))
			return ;
	}
	throw std::runtime_error("Invalid cgi");
}

void	AConfParser::parseAutoIndex(std::stringstream& ss, std::string& word)
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

void	AConfParser::parseLimitExcept(std::stringstream& ss, std::string& word)
{
	while (ss >> word)
	{
		if (isEnd(ss, word))
			return ;
		mHttpMethod.push_back(word);
	}
}

bool	AConfParser::isEnd(std::stringstream& ss, std::string& word)
{
	if (word != ";")
		return (false);
	else if (ss >> word)
		throw std::exception();
	else
		return (true);
}

void	AConfParser::parseClosedBracket(std::stringstream& ss, std::string& word)
{
	if (ss >> word)
		throw std::runtime_error("Wrong bracket format");
}
