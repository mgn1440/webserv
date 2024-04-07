#include <unordered_map>
#include <iostream>

int main(int argc, char *argv[], char *envp[])
{

	for (char **env = envp; *env != 0; env++)
	{
		std::string envStr = *env;
		std::cout << envStr << std::endl;
	}
	return (0);
}
