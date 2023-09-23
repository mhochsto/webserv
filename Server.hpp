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
		std::map<int, std::string> m_socketsIP;
		std::vector<t_server>	m_serverConfig;
		std::map<int, int> m_clientServerMap;

	public:
		Server( std::vector<t_server> serverConfig);
		~Server();
		void CreateServerSocket( t_server& server );
		void run( void );
		void addConnection( int serverFD );
		void handleRequest ( pollfd client );
		t_server findConfig(pollfd client);
		bool isServerSocket(int fd);
		void removeClient( pollfd client );
};

# define VALID_REQUESTS {"GET", "POST", "DELETE" }
# define REQUEST_FUNCTIONS {&Server::get, &Server::post, &Server::del}

#endif