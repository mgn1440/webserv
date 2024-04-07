#ifndef PARSE_UTILS_HPP
# define PARSE_UTILS_HPP

# include <string>
# include <vector>

/* ConfigHandler */
void exitWithError(const std::string& msg);
std::string seperateMetaChar(std::string& line, std::string set);

/* Http */
void trim(std::string& str, const std::string& del);
std::vector<std::string> split(const std::string& str, std::string delim);
bool checkCRLF(std::string& str);
void refreshBuffer(std::string& buf, size_t& size);

bool isDigits(const std::string& str);
bool isAlphas(const std::string& str);
bool isAlnums(const std::string& str);

#endif
