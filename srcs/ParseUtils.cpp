#include "ParseUtils.hpp"

std::string	chkAndSeperateMetaChar(std::string& line, std::string set)
{
	size_t	i = 0;
	size_t	len = 0;
	line = line.substr(0, (int)line.find('#'));
	for (char c = set[i]; i < set.length(); i ++, c = set[i])
	{
		size_t	j = line.find(c);
		while (j != std::string::npos)
		{
			len ++;
			line.replace(j, 1, " " + std::string(1, c) + " ");
			j = line.find(c, j + 2);
		}
	}
	if (len > 1)
		exitWithError("to many meta character");
	return line;
}

void	exitWithError(const std::string& msg)
{
	std::cerr << "Error: " << msg << std::endl;
	exit(1);
}

bool isWhitespaces(const std::string &str)
{
    return str.find_first_not_of("\r\v\n\t\f") == std::string::npos;
}

std::string	chkAndSeperateMetaChar(std::string line, std::string set)
{
	size_t	i = 0;
	size_t	len = 0;
	for (char c = set[i]; i < set.length(); i ++, c = set[i])
	{
		size_t	j = line.find(c);
		while (j != std::string::npos)
		{
			len ++;
			line.replace(j, 1, " " + std::string(1, c) + " ");
			j = line.find(c, j + 2);
		}
	}
	if (len > 1)
		exitWithError(" to many meta character");
	return line;
}
