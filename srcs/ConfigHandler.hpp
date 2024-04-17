#pragma once

#ifndef  CONFIG_HANDLER
# define CONFIG_HANDLER

# include <string>
# include <map>
# include <set>
# include <deque>
# include "Resource.hpp"


class Server;
class Response;

class ConfigHandler
{
public:
    typedef std::pair<int, std::string> serverInfo;

    static void MakeConfigHandler(const std::string& confPath);
    static ConfigHandler& GetConfigHandler();

    size_t GetMaxSize(int port, std::string& serverName, std::string& URI);
    std::set<int>& GetPorts();
    std::deque<Response> GetResponseOf(std::vector<struct Request> requests);
    std::string GetContentType(const std::string& URI);
    std::string GetABSPath(int port, const std::string& serverName ,const std::string& URI);
    std::string IsCGI(const std::string& ContentType);
    struct Resource GetResource(int port, const std::string& serverName, const std::string& URI);
    std::string GetServerName(const std::string& URI);

    // for DEBUG
    void PrintAll();
    void PrintServInfo(serverInfo info);
    ~ConfigHandler();
private:
    static ConfigHandler* configHandler;
    ConfigHandler();
    ConfigHandler(const std::string& confPath);
    ConfigHandler(const ConfigHandler& rhs);
    ConfigHandler& operator=(const ConfigHandler& rhs);

    void parseConfig(const std::string& confPath);
    bool isEnd(std::stringstream& ss, std::string& word);
    void parseClosedBracket(std::stringstream& ss, std::string& word);
    void chkValidFormOf(std::string& type, std::string& form);
    void parseTypeLine(std::stringstream& ss, std::string& word);
    void createServer(std::stringstream& ss, std::ifstream& confFile);
    void createTypes(std::stringstream& ss, std::ifstream& confFile);

    std::map<std::string, std::string> mTypeMap;
    std::map<serverInfo, Server> mServerMap;
    std::set<int> mPortSet;
};

#endif
