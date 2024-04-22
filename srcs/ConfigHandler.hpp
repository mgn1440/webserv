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
    static void MakeConfigHandler(const std::string& confPath);
    static ConfigHandler& GetConfigHandler();

    size_t GetMaxSize(int port, std::string& URI);
    std::set<int>& GetPorts();
    std::string GetContentType(const std::string& URI);
    std::string GetABSPath(int port, const std::string& URI);
    struct Resource GetResource(int port, const std::string& URI);

    // for DEBUG
    void PrintAll();
    void PrintServInfo(int port);
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
    std::map<int, Server> mServerMap;
    std::set<int> mPortSet;
};

#endif
