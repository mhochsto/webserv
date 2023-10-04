#ifndef DATASTRUCTS_HPP
# define DATASTRUCTS_HPP

#include "DefaultConfig.hpp"

typedef std::map<std::string, std::vector<std::string> > vectorMap;
typedef std::map<std::string, vectorMap > vectorMapMap; 
typedef std::map<std::string, std::string> stringMap;

enum chunkStatus {BadRequest,ChunkRecieved, Complete, recvError };

typedef struct s_config {

    /* Default (& minimal) Input for Server-config. */
    /* Macros defined in DefaultConfig.hpp */
    s_config() {
	    Data["port"].push_back(SERVER_LISTEN);
	    Data["hostname"].push_back(SERVER_HOST);
	    Data["client_max_body_size"].push_back(SERVER_CLIENT_MAX_BODY_SIZE);
	    Data["root"].push_back(SERVER_ROOT);
	    Data["index"].push_back(SERVER_INDEX);
	    Data["400"].push_back(SERVER_ERROR_PAGE_400);
	    Data["error_page"].push_back(SERVER_ERROR_PAGE_400);
	    Data["error_page"].push_back(SERVER_ERROR_PAGE_405);
	    Data["error_page"].push_back(SERVER_ERROR_PAGE_500);
	    Data["error_page"].push_back(SERVER_ERROR_PAGE_502);
	    Data["error_page"].push_back(SERVER_ERROR_PAGE_503);
	    Data["error_page"].push_back(SERVER_ERROR_PAGE_500);
        fd = 0;
    }

    vectorMap							redirects;
    vectorMap                           errorPages;
    vectorMap                           Data;
    vectorMapMap                        locations;
    int                                 fd;
} t_config;

typedef struct s_client {
	s_client(){
		fd = 0;
		serverFD = 0;
		chunkSizeLong = 0;
	}
	int fd;
	int serverFD;
	std::string ip;
	std::string header;
	std::string body;
	std::string chunk;
	t_config config;
	vectorMap location;
	long chunkSizeLong;
	chunkStatus chunkState;
} t_client;

#endif