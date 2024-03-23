#ifndef PARSEUTILS_HPP
# define PARSEUTILS_HPP

std::string trim(std::string& str);
std::vector<std::string> split(const std::string& str, std::string delim);
bool checkCRLF(std::string& str);
void refreshBuffer(std::string& buf, int& size);

#endif
