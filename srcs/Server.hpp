#pragma once

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <map>
# include <set>
# include <vector>

# include "Location.hpp"

class Server {
public :
	Server();
	Server&		operator = (const Server& rhs);
	~Server();

	void	PrintInfo();
	void	ParseLine(std::string  line);
	void	PutIn(std::map<int, Server>& rhs);
private :
	Server(const Server& rhs);
	
	void	parseLocation();
	void	parseListen(std::stringstream& ss, std::string& word);
	void	parseServerName(std::stringstream& s,, std::string& word);
	void	parseErrorPage();
	void	parseClientMaxSize();
	void	parseRoot();
	void	parseAutoIndex();
	void	parseLimitExcept();
	void	parseIndex();

	int	classifySymbol(std::string symbol);
	int	mChkBracket;
	
	bool		mbInLocation;
	std::set<int>				mPort;
	std::vector<std::string>	mServerName;
	std::map<int, std::string>	mErrorPage;
	long long					mMaxSize[3];
	std::vector<std::string>	mHttpMethod;
	std::string					mRoot;
	std::set<std::string>		mIndex;
	bool						mbAutoIndex;
	
	std::map<std::string, Location>	mLocationMap;

};

#endif