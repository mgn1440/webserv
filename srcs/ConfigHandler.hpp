#pragma once

#ifndef  CONFIG_HANDLER
# define CONFIG_HANDLER

# include <string>
# include <map>
# include <deque>


class Server;
class Response;

class ConfigHandler
{
public:
    typedef std::pair<int, std::string> serverInfo;
    
    static void MakeConfigHandler(const std::string& confPath);
    static ConfigHandler& GetConfigHandler();

    const size_t* GetMaxSizes(int port, std::string serverName);
    std::deque<Response> GetResponseOf(std::vector<struct Request> requests);
    
    // for DEBUG
    void PrintAll();
    void PrintServInfo(serverInfo info);
private:
    static ConfigHandler* configHandler;
    ConfigHandler();
    ConfigHandler(const std::string& confPath);
    ConfigHandler(const ConfigHandler& rhs);
    ConfigHandler& operator=(const ConfigHandler& rhs);
    ~ConfigHandler();

    void parseConfig(const std::string& confPath);
    bool isEnd(std::stringstream& ss, std::string& word);
    void parseClosedBracket(std::stringstream& ss, std::string& word);
    void chkValidFormOf(std::string& type, std::string& form);
    void parseTypeLine(std::stringstream& ss, std::string& word);
    void createServer(std::stringstream& ss, std::ifstream& confFile);
    void createTypes(std::stringstream& ss, std::ifstream& confFile);

    std::map<std::string, std::string> mTypeMap;
    std::map<serverInfo, Server> mServerMap;
};

#endif