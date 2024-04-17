#ifndef CONVERTUTILS_HPP
# define CONVERTUTILS_HPP
# include <cstdlib>
# include <string>

size_t convertNum(std::string num);
size_t convertHex(std::string num);
void percentDecoding(std::string& URI);
std::string intToString(size_t num);
std::string headerToCGIVar(const std::string& str);

#endif