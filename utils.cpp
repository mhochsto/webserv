#include "utils.hpp"
#include "Error.hpp"
/* used to sort biggest to smallest */
static bool compareLength(std::string& str1, std::string& str2){ return (str1.length() > str2.length());}


std::string closestMatchingLocation( locationMap locMap, std::string path){
    std::vector<std::string> locMapNames;
    for (locationMap::iterator it = locMap.begin(); it != locMap.end(); it++){
        locMapNames.push_back(it->first);
    }
    path = "." + path;
    std::sort(locMapNames.begin(), locMapNames.end(), compareLength);
    do {
        for (std::vector<std::string>::iterator it = locMapNames.begin(); it != locMapNames.end(); it++){
            if (*it == path){
                return path;
            }
        }
        path.erase(path.find_last_of('/'));
        if (path == "."){
            return "./";
        }
    } while (!path.empty());
    return path;
}

std::string timestamp(void){
	time_t rawtime;
	struct tm * tm_localTime;
	char buffer[200];

	time (&rawtime);
	tm_localTime = localtime (&rawtime);    
	strftime(buffer, 200,"Date: %a, %d %b %G %T %Z\n",tm_localTime);

	std::string str(buffer);
	return (str.c_str());
}


/* remove preceding Whitespace, first word and trailing whitespace from str */
/* return first Word as new string*/
std::string getFirstWord(std::string& str) {
	if (str.find_first_of(WHITESPACE) == 0){
		str.erase(0, str.find_first_not_of(WHITESPACE));
	}
	std::string word = str.substr(0, str.find_first_of(WHITESPACE));
	str.erase(0, str.find_first_of(WHITESPACE));
	str.erase(0, str.find_first_not_of(WHITESPACE));	
	return (word);
}

std::string convertIPtoString(unsigned long ip){
    std::stringstream sstream;
	sstream << (int)((ip >> 24) & 0xFF);
	sstream << ".";
    sstream << (int)((ip >> 16) & 0xFF);
	sstream << ".";
    sstream << (int)((ip >> 8) & 0xFF);
	sstream << ".";
    sstream << (int)(ip & 0xFF);
	return sstream.str();
}
/* wip */
void    print(printState state, std::string msg){
    std::cout << msg << std::endl;
    (void)state;
}

std::vector<std::string> convertStringtoVector(std::string str, std::string delimiter){
	std::vector<std::string> v;
	do {
		str.erase(0, str.find_first_not_of(delimiter));
		v.push_back(str.substr(0, str.find_first_of(delimiter)));
		str.erase(0, v.back().length() + 1);
	} while (!str.empty());
	return v;
}

void	removeFirstWord(std::string& str) {
	str.erase(0, str.find_first_not_of(WHITESPACE));
	str.erase(0, str.find_first_of(WHITESPACE));
}

/* remove one trailing '/' && add one '/' if necessary -> perfect outcome == "/str" */
void formatPath(std::string& str){
	if (str != "/" && str.at(str.length() - 1) == '/')
		str.resize(str.length() - 1);
	if (str.at(0) != '/')
		str = "/"+ str;
}
