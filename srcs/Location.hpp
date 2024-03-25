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
	Location(std::ifstream& confFile);
	Location& operator=(const Location& rhs);
	~Location();

	void ParseLocation(std::string& line);
private:
	void parse(std::ifstream& confFile);
	Location(const Location& rhs);

	// TODO: cgi var needed
};

#endif