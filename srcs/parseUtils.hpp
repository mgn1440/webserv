#ifndef PARSEUTILS_HPP
# define PARSEUTILS_HPP

void trim(std::string& str, const std::string& del);
std::vector<std::string> split(const std::string& str, std::string delim);
bool checkCRLF(std::string& str);
void refreshBuffer(std::string& buf, size_t& size);

#endif
