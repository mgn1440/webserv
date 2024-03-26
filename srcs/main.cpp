#include <iostream>
#include <string>

# include <fstream>
# include <sstream>

#include "ConfigHandler.hpp"
#include "ParseUtils.hpp"

// #include "HttpResponse.hpp"

/** // TODO: 
 * Read one line				(server{listen 80; server_name hello;	# Choose the port and host of each server)
 * Divide annotation			(server{listen 80; server_name hello;)
 * seperate METACHAR("{}", ";")	(server { listen 80 ; server_name hello ; )
 * 
*/

void	configHandlerFactory(const std::string confPath)
{
	std::cout << confPath << std::endl;
	ConfigHandler configHandler(confPath);
	while (true)
	{
		int port;
		std::cout << "Input port num: ";
		std::cin >> port;
		configHandler.PrintInfo(port);
	}
}

int	main(int ac, char** av)
{
	try
	{
		if (ac > 2)
			std::cerr << "Usage: " << av[0] << " <conf dir>(optional)" << std::endl;
	}
	catch (std::exception& e) {
		exitWithError(std::string(e.what()));
	}

}