#ifndef UTILS_HPP
# define UTILS_HPP

#include <algorithm>
#include "Config.hpp"

#define RED "\033[1;31m"
#define RESET "\033[0m"
enum printState {Error, req, resp, Notification};

std::string closestMatchingLocation( vectorMapMap locMap, std::string path);
std::string timestamp(void);
std::string getFirstWord(std::string str);
std::string convertIPtoString(unsigned long ip);
void    print(printState state, std::string msg);
void    formatPath(std::string& str);
void	removeFirstWord(std::string& str);
std::vector<std::string> convertStringtoVector(std::string str, std::string delimiter);
#endif