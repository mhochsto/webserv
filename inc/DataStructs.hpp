#ifndef DATASTRUCTS_HPP
# define DATASTRUCTS_HPP

#include "DefaultConfig.hpp"
#include <map>
#include <vector>
#include <string>
#include <sstream>

# include <poll.h>

class Request;
class CgiHandler;

typedef struct s_location {
	s_location() {
		autoIndex = false;
		clientMaxBodySize = UNSET;
	}
	bool						autoIndex;
	ssize_t						clientMaxBodySize;
	std::string					index;
	std::string					proxyPass;
	std::string					path;
	std::string					root;
	std::string					cgiScript;
	std::vector <std::string>	allowedCgiExtensions;
	std::vector <std::string>	allowedMethods;
} t_location;

typedef std::map<std::string, t_location> locationMap; 
typedef std::map<std::string, std::string> stringMap;

/* default server name muss im parsing noch entfernt werden */
typedef struct s_config {
	/* alles mit initialiser list ?*/
    s_config() : fd(UNSET) {
		port = SERVER_LISTEN;
		clientMaxBodySize = SERVER_CLIENT_MAX_BODY_SIZE;
		root = SERVER_ROOT;
		serverName = SERVER_LOCALHOST;
		errorPages["400"] = SERVER_ERROR_PAGE_400;
		errorPages["404"] = SERVER_ERROR_PAGE_404;
		errorPages["405"] = SERVER_ERROR_PAGE_405;
		errorPages["500"] = SERVER_ERROR_PAGE_500;
		errorPages["502"] = SERVER_ERROR_PAGE_502;
		errorPages["503"] = SERVER_ERROR_PAGE_503;
		errorPages["505"] = SERVER_ERROR_PAGE_505;
		locations["/"];
    }
    int                                 fd;
	ssize_t								port;
	ssize_t								clientMaxBodySize;
	std::string							root;
	std::string							index;
	std::string							serverName;
    stringMap							redirects;
    stringMap							errorPages;
    locationMap							locations;
} t_config;

enum RecieveState { header, body, chunk, done};

typedef struct s_client {

	s_client(){
		fd = 0;
		serverFD = 0;
		chunkSizeLong = 0;
		request = NULL;
		cgi = NULL;
		activeCGI = false;
	}

	bool					activeCGI;
	int						CgiPid;
	int 					fd;
	int 					serverFD;
	long 					chunkSizeLong;
	long 					exptectedBodySize;
	std::string 			ip;
	std::string 			header;
	std::string 			body;
	std::string 			chunk;
	RecieveState			recieving;
	std::vector<pollfd> 	*socketVector;
	t_config 				config;
	t_location				location;
	Request 				*request;
	CgiHandler				*cgi;

} t_client;



std::ostream	&operator<<(std::ostream &os, const t_config &rhs); // written in the utils.cpp

#endif