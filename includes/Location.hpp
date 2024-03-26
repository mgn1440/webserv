#pragma once

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>

# include <set>
# include <map>
# include <vector>

# include "STLUtils.hpp" // to debug
# include "AConfParser.hpp"

class Location : public AConfParser
{
public:
	Location();
	Location(std::ifstream& confFile);
	Location& operator=(const Location& rhs);
	Location(const Location& src);
	~Location();

	void PrintInfo(); //debug
	void ParseLocation(std::string& line);
private:
	void parse(std::ifstream& confFile);

	// TODO: cgi var needed
};

#endif