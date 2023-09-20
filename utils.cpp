#include "utils.hpp"

/* used to sort biggest to smallest */
static bool compareLength(std::string& str1, std::string& str2){ return (str1.length() > str2.length());}

std::string closestMatchingLocation( std::map<std::string, t_location> locMap, std::string path){
    std::vector<std::string> locMapNames;
    for (std::map<std::string, t_location>::iterator it = locMap.begin(); it != locMap.end(); it++){
        locMapNames.push_back(it->first);
    }
    std::sort(locMapNames.begin(), locMapNames.end(), compareLength);
    do {
        if (path.find_first_of('/') == path.find_last_of('/'))
            return path;
        for (std::vector<std::string>::iterator it = locMapNames.begin(); it != locMapNames.end(); it++){
            if (*it == path)
                return path;
        }
        path.erase(path.find_last_of('/'));
    } while (!path.empty());
    return path;
}