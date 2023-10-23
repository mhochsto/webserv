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
#include <numeric>

# include "Config.hpp"

# define CLIENT_MAX 200
# define TIMEOUT 5000 //ms

class Config;

class Server {
	public:
		Server( std::vector<t_config> serverConfig );
		~Server();
		void run( void );
		void removeClient( t_client& client );
		void CreateServerSocket( t_config& server );
		bool isServerSocket(int fd);
	private:
		std::vector<pollfd> 	m_sockets;
		std::vector<t_config>	m_serverConfig;
		std::map<int, t_client> m_clients;

		void 		CreateServerSocket( t_config& server );
		void 		setConfig( t_client& client );
		bool 		isServerSocket(int fd);
		void 		removeClient( t_client& client );

		void 		addConnection( int serverFD );
		ssize_t 	recvFromClient(std::string&data, t_client& client);
		void 		handleRequest( t_client& client );
		void 		setRecieveState(t_client& client);
		int 		setChunkSize( t_client& client );
		void 		saveChunk(t_client& client);
		void 		sendResponse(t_client& client);

	public:
		Server( std::vector<t_config> serverConfig );
		void		run( void );
		~Server();
};

#endif