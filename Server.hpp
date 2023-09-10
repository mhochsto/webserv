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
# define HTTP_HEADER_LIMIT 8192
# define REQUEST 0
# define PATH 1

class Server {
	private:
		int					m_httpBodyLimit;
		pollfd 				m_serverSocket;
		std::vector<pollfd> m_sockets;	
	public:
		Server();
		void run( void );
		void addConnection( void );
		void respond ( int clientIndex );
};

# define VALID_REQUESTS {"GET", "POST", "DELETE" }
# define REQUEST_FUNCTIONS {&Server::get, &Server::post, &Server::del}

#endif