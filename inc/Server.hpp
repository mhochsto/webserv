#ifndef SERVER_HPP
# define SERVER_HPP

# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <netdb.h>
# include <unistd.h>
# include <poll.h>
# include <vector>
# include <iostream>
# include <exception>
# include <cstring>
# include <cerrno>
# include <fstream>
# include <sstream>
# include <algorithm>
# include <numeric>
# include <csignal>
# include <ctime>
# include "Config.hpp"

# define CLIENT_MAX 10000

/* Note that REQUEST_TIMEOUT should be >CGI_TIMOUT */
# define REQUEST_TIMEOUT 2 //s
# define CGI_TIMEOUT 1 // s
class Config;

class Server {
	public:
		Server( std::vector<t_config> serverConfig );
		~Server();
		void run( void );
		void removeClient( t_client& client );
		void CreateServerSocket( t_config& server );
		bool isServerSocket(int fd);
		t_client* findClient(int pipeFd);
	private:
		std::vector<pollfd> 	m_sockets;
		std::vector<t_config>	m_serverConfig;
		std::map<int, t_client> m_clients;

		t_config&	setConfig( int serverFD );
		void 		addConnection( int serverFD );
		ssize_t 	recvFromClient(std::string&data, t_client& client);
		void 		recieveRequest( t_client& client );
		void 		setRecieveState(t_client& client);
		int 		setChunkSize( t_client& client );
		void 		saveChunk(t_client& client);
		void 		sendResponse(t_client& client);
		void 		removeFdFromSocketVec( int fd );
		void 		handleCgiSockets( pollfd& pollfd );
		void 		handleClient(pollfd& client);
		bool		isClientSocket(pollfd& pollfd){ return pollfd.revents != 0 && m_clients.find(pollfd.fd) != m_clients.end();}
		void		removeCgiSockets(t_client *cgiClient);
		static void		sigIntHandler(int signal );
};

#endif