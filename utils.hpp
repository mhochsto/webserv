#ifndef UTILS_HPP
# define UTILS_HPP

#include <algorithm>
#include "Config.hpp"


std::string closestMatchingLocation( std::map<std::string, t_location> locMap, std::string path);
std::string timestamp(void);
std::string getFirstWord(std::string str);
std::string convertIPtoString(unsigned long ip);

#endif