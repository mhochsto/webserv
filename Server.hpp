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
		pollfd 					m_serverSocket;
		std::vector<pollfd> 	m_sockets;
		std::map<int, std::string> m_socketsIP;
		std::vector<t_server>	m_servers;
		t_server                m_serv;

	public:
		Server( Config config );
		void run( void );
		void addConnection( void );
		void respond ( pollfd client );
		std::string convertIPtoString(unsigned long ip);
};

# define VALID_REQUESTS {"GET", "POST", "DELETE" }
# define REQUEST_FUNCTIONS {&Server::get, &Server::post, &Server::del}

#endif