#include <cstring>
#include <stdexcept>
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
		std::string confPath("./conf/default.conf");
		if (argc == 2)
			confPath = argv[1];
		WebServer webServer(confPath);
		webServer.SetTCP();
		webServer.RunTCP();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
}
