#include <cstring>
#include <stdexcept>
#include <iostream>
#include <string>
#include "../includes/WebServer.hpp"

int main(int argc, char *argv[])
{
	try
	{
		if (argc > 2)
			throw std::invalid_argument("Require one or two argument");
		WebServer webServer;
		if (argc == 2)
		{
			WebServer tempServer(argv[1]);
			webServer = tempServer;
		}
		webServer.SetTCP();
		webServer.RunTCP();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}
