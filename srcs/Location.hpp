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
	~Location();

	void	ParseLine(std::string& line);
private:
	Location& operator=(const Location& rhs);
	Location(const Location& rhs);

	// TODO: cgi var needed
};

#endif