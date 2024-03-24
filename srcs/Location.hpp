#pragma once

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>

# include <set>
# include <map>
# include <vector>

# include "STLUtils.hpp"

class Location
{
public :
	Location();
	~Location();

	void	ParseLine(std::string& line);
private :
	Location&	operator = (const Location& rhs);
	Location(const Location& rhs);
	std::vector<std::string>	mHttpMethod;
	std::string					mRoot;
	std::set<std::string>		mIndex;

	bool						mbAutoIndex;
};

#endif