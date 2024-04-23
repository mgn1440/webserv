#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <map>
# include <set>
# include <vector>
# include "Location.hpp"
# include "AConfParser.hpp"
# include "Resource.hpp"

class Server : public AConfParser
{
public:
	typedef std::pair<int, std::string> serverInfo;
	
	Server();
	Server(std::ifstream& confFile);
	Server&	operator=(const Server& rhs);
	~Server();
	void PrintInfo();
	void PutIn(std::map<int, Server>& rhs);
	size_t GetMaxSize(std::string &URI);
	std::set<int>& GetPorts();
	struct Resource GetResource(std::string URI);
	std::string GetABSPath(const std::string& URI);
private:
	Server(const Server& rhs);
	void parse(std::ifstream& confFile);
	void parseLocation(std::ifstream& confFile, std::stringstream& ss, std::string& word);
	void parseListen(std::stringstream& ss, std::string& word);
	void parseServerName(std::stringstream& ss, std::string& word);
	void parseErrorPage(std::stringstream& ss, std::string& word);
	void parseUpload(std::stringstream& ss, std::string& word);
	std::string searchLocationPath(const std::string& URI);
	
	bool mbIsDuplicatedUpload;
	std::string mUpload;
	std::set<int> mPort;
	std::vector<std::string> mServerName;
	std::map<int, std::string> mErrorPage;
	std::map<std::string, Location> mLocationMap;
};

#endif