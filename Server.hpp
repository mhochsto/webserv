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


# define HTTP_HEADER_LIMIT 8192
# define CLIENT_MAX 200

class Config;

class Server {
	private:
		pollfd 					m_serverSocket;
		std::vector<pollfd> 	m_sockets;
		std::vector<t_server>	m_servers;
		t_server                m_serv;

	public:
		Server( Config config );
		void run( void );
		void addConnection( void );
		void respond ( int clientIndex );
};

# define VALID_REQUESTS {"GET", "POST", "DELETE" }
# define REQUEST_FUNCTIONS {&Server::get, &Server::post, &Server::del}

#endif