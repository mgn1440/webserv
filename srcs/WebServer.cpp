#pragma  once

#ifndef ConfigHandler
# define ConfigHandler

# include <fstream>

# include <map>

class Location
{
public :
	Location(std::ifstream& ifs);
private : 

};

class Server
{
public :
	Server(std::ifstream& ifs); // => ParseServer
	Server&		operator = (const Server& rhs);
	~Server();

	void	PrintInfo();
	void	ParseServer(std::ifstream& ifs);
	void	PutIn(std::map<int, Server>& rhs);
private :
	Server(const Server& rhs);

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


class ConfigHandler
{
public :
	ConfigHandler(std::string path);
	~ConfigHandler();
	/**
	 * TODO:
	 * find server block and deliver them into server class.
	 * save server class into mServerMap 
	*/
	ParseConfig(std::string path);
private :
	ConfigHandler(const ConfigHandler& rhs);
	ConfigHandler&	operator = (const ConfigHandler& rhs);

	std::map<int, Server> mServerMap;
};

#endif