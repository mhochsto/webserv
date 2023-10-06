#ifndef DATASTRUCTS_HPP
# define DATASTRUCTS_HPP

#include "DefaultConfig.hpp"
#include <map>
#include <vector>
#include <string>
#include <sstream>

enum chunkStatus {BadRequest,ChunkRecieved, Complete, recvError };

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
	std::vector <std::string>	allowedCgiExtensions;
	std::vector <std::string>	allowedMethods;
} t_location;

typedef std::map<std::string, t_location> locationMap; 
typedef std::map<std::string, std::string> stringMap;

/* default server name muss im parsing noch entfernt werden */
typedef struct s_config {
    s_config() {

        fd = UNSET;
		port = SERVER_LISTEN;
		clientMaxBodySize = SERVER_CLIENT_MAX_BODY_SIZE;
		root = SERVER_ROOT;
		serverName.push_back(SERVER_NAME);
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
	std::vector <std::string>			serverName;
    stringMap							redirects;
    stringMap							errorPages;
    locationMap							locations;
} t_config;

typedef struct s_client {
	s_client(){
		fd = 0;
		serverFD = 0;
		chunkSizeLong = 0;
	}
	int fd;
	int serverFD;
	long chunkSizeLong;
	std::string ip;
	std::string header;
	std::string body;
	std::string chunk;
	t_config config;
	t_location location;
	chunkStatus chunkState;
} t_client;

#endif