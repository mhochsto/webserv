#include "utils.hpp"
#include "Error.hpp"
#include "DataStructs.hpp"

/* used to sort biggest to smallest */
static bool compareLength(std::string& str1, std::string& str2){ return (str1.length() > str2.length());}


std::string closestMatchingLocation( locationMap locMap, std::string path){
    std::vector<std::string> locMapNames;
    for (locationMap::iterator it = locMap.begin(); it != locMap.end(); it++){
        locMapNames.push_back(it->first);
    }
    std::sort(locMapNames.begin(), locMapNames.end(), compareLength);
    do {
        for (std::vector<std::string>::iterator it = locMapNames.begin(); it != locMapNames.end(); it++){
            if (*it == path){
                return path;
            }
        }
        path.erase(path.find_last_of('/'));
        if (path.empty()){
            return "/";
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


void closePollfds( std::vector<pollfd> pollfds ){
	for (std::vector<pollfd>::iterator it = pollfds.begin(); it != pollfds.end(); ++it){
			close(it->fd);
	}
}

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
	sstream << (int)( ip & 0xFF);
	sstream << ".";
    sstream << (int)((ip >> 8) & 0xFF);
	sstream << ".";
    sstream << (int)((ip >> 16) & 0xFF);
	sstream << ".";
    sstream << (int)((ip >> 24) & 0xFF);
	return sstream.str();
}
/* wip */
void    print(printState state, std::string msg){
    std::cout << msg << std::endl;
	(void)msg;
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

void formatPath(std::string& str){
	if (str != "/" && str.at(str.length() - 1) == '/')
		str.resize(str.length() - 1);
	if (str.at(0) != '/')
		str = "/"+ str;
}
/*
std::ostream	&operator<<(std::ostream &os, const t_config &rhs) {
	os << "Whats in the Config Struct named serv!" << std::endl;
	os << "\tserv.fd: " << rhs.fd << std::endl;
	os << "\tserv.port: " << rhs.port << std::endl;
	os << "\tserv.root: " << rhs.root << std::endl;
	os << "\tserv.index: " << rhs.index << std::endl;
	os << "\teverything in the serv.serverName vector: " << std::endl;
	for (std::vector<std::string>::const_iterator it = rhs.serverName.begin(); it != rhs.serverName.end(); it++) {
		os << "\t\t" << *it << std::endl;
	}
	os << "\teverything in the serv.redirects map: " << std::endl;
	for (stringMap::const_iterator it = rhs.redirects.begin(); it != rhs.redirects.end(); it ++) {
		os << "\t\t" << it->first << ": " << it->second << std::endl;
	}
	os << "\teverything in the serv.errorPages map: " << std::endl;
	for (stringMap::const_iterator it = rhs.errorPages.begin(); it != rhs.errorPages.end(); it ++) {
		os << "\t\t" << it->first << ": " << it->second << std::endl;
	}
	os << "\teverything in the serv.locations map: " << std::endl;
	for (locationMap::const_iterator it = rhs.locations.begin(); it != rhs.locations.end(); it ++) {
		os << "\t\t" << "BASIC INFOS:" << it->first << std::endl;
		os << "\t\t" << "AutoIndex: " << it->second.autoIndex << std::endl;
		os << "\t\t" << "ClientMaxBodySize: " << it->second.clientMaxBodySize << std::endl;
		os << "\t\t" << "Index: " << it->second.index << std::endl;
		os << "\t\t" << "ProxyPass: " << it->second.proxyPass << std::endl;
		os << "\t\t" << "Path: " << it->second.path << std::endl;
		os << "\t\t" << "Everything in the serv.location.allowedCgiExtensions XD" << std::endl;
		for (std::vector<std::string>::const_iterator itCgi = it->second.allowedCgiExtensions.begin(); itCgi != it->second.allowedCgiExtensions.end(); itCgi++) {
			os << "\t\t\t" << *itCgi << std::endl;
		}
		os << "\t\t" << "Everything in the serv.location.allowedMethods XD" << std::endl;
		for (std::vector<std::string>::const_iterator itMethods = it->second.allowedMethods.begin(); itMethods != it->second.allowedMethods.end(); itMethods++) {
			os << "\t\t\t" << *itMethods << std::endl;
		}
	}
	return (os);
}
*/
