#pragma once

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>

# include <set>
# include <map>
# include <vector>

# include "STLUtils.hpp" // to debug
# include "AConfParser.hpp"
# include "Resource.hpp"

class Location : public AConfParser
{
public:
	Location();
	Location(std::ifstream& confFile);
	Location& operator=(const Location& rhs);
	Location(const Location& src);
	~Location();

	void GetRoot(std::string& path);
	void SetResource(struct Resource& res);
	void PrintInfo(); //debug
	void ParseLocation(std::string& line);
private:
	void parse(std::ifstream& confFile);

	// TODO: cgi var needed
};

#endif