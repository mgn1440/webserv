#pragma once

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <map>
# include <set>
# include <vector>

# include "Location.hpp"
# include "AConfParser.hpp"

class Server : public AConfParser
{
public:
	Server();
	Server(std::ifstream& confFile);
	Server&	operator=(const Server& rhs);
	~Server();
	void	PrintInfo();
	void	ParseLine(std::string  line);
	void	PutIn(std::map<int, Server*>& rhs);
private:
	Server(const Server& rhs);
	void	parse(const std::string& confPath);
	void	parseLocation(std::stringstream& ss, std::string& word);
	void	parseClosedBracket();
	void	parseListen(std::stringstream& ss, std::string& word);
	void	parseServerName(std::stringstream& ss, std::string& word);
	void	parseErrorPage(std::stringstream& ss, std::string& word);
	void	parseClientMaxSize(std::stringstream& ss, std::string& word);

	// int	classifySymbol(std::string symbol);	
	bool							mbInLocation;
	long long						mMaxSize[3];
	int								mChkBracket;
	std::set<int>					mPort;
	std::vector<std::string>		mServerName;
	std::map<int, std::string>		mErrorPage;
	std::map<std::string, Location*>	mLocationMap;
};

#endif