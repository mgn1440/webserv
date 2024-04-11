#include <unordered_map>
#include <iostream>
#include "ConfigHandler.hpp"
#include "AConfParser.hpp"
#include "HttpHandler.hpp"
#include "parseUtils.hpp"
#include "Response.hpp"
#include "Request.hpp"
#include "WebServ.hpp"

int main(int argc, char *argv[], char *envp[])
{

	try
	{
		if (argc > 2)
			throw std::runtime_error("Bad argc");
		if (argc == 2)
			ConfigHandler::MakeConfigHandler(argv[1]);
		else
			ConfigHandler::MakeConfigHandler("./conf/default.conf");
		std::vector<std::string> envList;
		for (char **env = envp; *env != 0; env++)
		{
			envList.push_back(*env);
		}
		WebServ webServ(ConfigHandler::GetConfigHandler().GetPorts(), envList);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	return (0);
}
