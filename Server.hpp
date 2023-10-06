#ifndef SERVER_HPP
# define SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <exception>
#include <cstring>
# include <poll.h>
#include <cerrno>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <algorithm>
# include <numeric>

# include "Config.hpp"


# define CLIENT_MAX 200
# define TIMEOUT 5000 //ms

class Config;


class Server {
	private:
		std::vector<pollfd> 	m_sockets;
		std::vector<t_config>	m_serverConfig;
		std::map<int, t_client> m_clients;

	public:
		Server( std::vector<t_config> serverConfig );
		~Server();
		void CreateServerSocket( t_config& server );
		void run( void );
		void addConnection( int serverFD );
		int handleRequest ( t_client& client );
		void setConfig( t_client& client );
		bool isServerSocket(int fd);
		void removeClient( t_client& client );

		void 		sendResponse(t_client& client, std::string status );
		chunkStatus	recvChunks(t_client& client);	
		ssize_t 	recvHeader(t_client& client);
		bool 		headerFullyRecieved(t_client& client);

};

# define VALID_REQUESTS {"GET", "POST", "DELETE" }
# define REQUEST_FUNCTIONS {&Server::get, &Server::post, &Server::del}

#endif