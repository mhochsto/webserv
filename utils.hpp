#ifndef UTILS_HPP
# define UTILS_HPP

#include "Config.hpp"
#include <algorithm>

std::string closestMatchingLocation( std::map<std::string, t_location> locMap, std::string path);
std::string timestamp(void);

#endif