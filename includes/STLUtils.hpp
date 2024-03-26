# pragma once

#ifndef STL_UTILS_HPP
# define STL_UTILS_HPP

# include <iostream>
# include <map>
# include <set>
# include <vector>

template <typename T>
void	printSet(std::set<T>& rhs)
{
	for (typename std::set<T>::iterator it = rhs.begin(); it != rhs.end(); it ++)
		std::cout << *it << ", ";
	std::cout << std::endl;
}

template <typename T, typename U>
void	printMap(std::map<T, U>& rhs)
{
	for (typename std::map<T, U>::iterator it = rhs.begin(); it != rhs.end(); it ++)
		std::cout << "key:" << it->first << " - val:" << it->second << ", ";
	std::cout << std::endl;
}

template <typename T>
void	printVec(std::vector<T>& rhs)
{
	for (typename std::vector<T>::iterator it = rhs.begin(); it != rhs.end(); it ++)
		std::cout << *it << ", ";
	std::cout << std::endl;
}

#endif