#include "AConfParser.hpp"

AConfParser::AConfParser()
{}

AConfParser::~AConfParser()
{}

AConfParser::AConfParser(const AConfParser& rhs)
{
	static_cast<void>(rhs);
}

AConfParser&	AConfParser::operator=(const AConfParser& rhs)
{
	static_cast<void>(rhs);
	return (*this);
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

bool	AconfParser::isEnd(std::stringstream& ss, std::string& word)
{
	if (word != ";")
		return (false);
	else if (ss >> word)
		throw std::exception();
	else
		return (true);
}