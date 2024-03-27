#ifndef PARSE_UTILS_HPP
# define PARSE_UTILS_HPP

# include <iostream>
# include <string>
# include <vector>

/* ConfigHandler */
void exitWithError(const std::string& msg);
std::string seperateMetaChar(std::string& line, std::string set);

/* Http */
std::string trim(std::string& str);
std::vector<std::string> split(const std::string& str, std::string delim);
bool checkCRLF(std::string& str);
void refreshBuffer(std::string& buf, int& size);

#endif
