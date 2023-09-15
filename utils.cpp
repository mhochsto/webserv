#include "utils.hpp"

static unsigned int levenstein(const std::string path, const std::string toMatch){
    if (path.size() == 0)
        return toMatch.size();
    if (toMatch.size() == 0)
        return path.size();
    if (path.at(0) == toMatch.at(0)){
        return levenstein(&path[1], &toMatch[1]);
    }
    return 1 + std::min(
        std::min(
            levenstein(path, toMatch.substr(1)),
            levenstein(path.substr(1), toMatch)
        ),  levenstein(path.substr(1), toMatch.substr(1)));
}

std::string closestMatchingLocation( std::map<std::string, t_location> locMap, std::string path){
    std::vector<std::string> locMapNames;
    for (std::map<std::string, t_location>::iterator it = locMap.begin(); it != locMap.end(); it++){
        locMapNames.push_back(it->first);
    }

    std::vector<unsigned int> diviationIndex;
    for (unsigned long i = 0; i < locMapNames.size(); i++){
        diviationIndex.push_back(levenstein(path, locMapNames[i]));
    }
    int smallestDiviationIndex = std::distance(diviationIndex.begin(), std::min_element(diviationIndex.begin(), diviationIndex.end()));
    return (locMapNames.at(smallestDiviationIndex));
}