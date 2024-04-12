#include <vector>
#include <string>
#include <iostream>

int main(int argc, char *argv[], char *envp[])
{

		std::vector<std::string> envList;
		for (char **env = envp; *env != 0; env++)
		{
			std::cout << *env << std::endl;
			envList.push_back(*env);
		}

	return (0);
}
