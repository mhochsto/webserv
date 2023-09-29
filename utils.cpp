#include "utils.hpp"
#include "Error.hpp"
/* used to sort biggest to smallest */
static bool compareLength(std::string& str1, std::string& str2){ return (str1.length() > str2.length());}

std::string closestMatchingLocation( std::map<std::string, t_location> locMap, std::string path){
    std::vector<std::string> locMapNames;
    for (std::map<std::string, t_location>::iterator it = locMap.begin(); it != locMap.end(); it++){
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


std::string getFirstWord(std::string str){
	str = str.substr(str.find_first_not_of(WHITESPACE));
	return (str.substr(0, str.find_first_of(WHITESPACE)));
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