
# include "Server.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Error.hpp"
# include "Config.hpp"

/* ###################################################### */

/* ###################################################### */


void Server::CreateServerSocket( t_server& server ){
	pollfd	serverSocket;
	std::memset(&serverSocket, 0, sizeof(serverSocket));

	serverSocket.events = POLLIN | POLLOUT;
	serverSocket.fd = (socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0));
	if (serverSocket.fd == -1){
		throw std::runtime_error(SYS_ERROR("socket"));
	}

	int on = 1;
	if (setsockopt(serverSocket.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
		close(serverSocket.fd);
		throw std::runtime_error(SYS_ERROR("setsockopt"));
	}

	sockaddr_in serverAddress;
	std::memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(server.port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverSocket.fd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1){
		close(serverSocket.fd);
		throw std::runtime_error(SYS_ERROR("bind"));
	}
	if (listen(serverSocket.fd, CLIENT_MAX)){
		close(serverSocket.fd);
		throw std::runtime_error(SYS_ERROR("listen"));
	}
	m_sockets.push_back(serverSocket);
	server.fd = serverSocket.fd;
}

Server::Server( std::vector<t_server> serverConfig): m_serverConfig(serverConfig){
	for (std::vector<t_server>::iterator it = m_serverConfig.begin(); it != m_serverConfig.end(); ++it){
		CreateServerSocket(*it);
	}
}

bool Server::isServerSocket(int fd){
	for(unsigned long i = 0; i < m_serverConfig.size(); ++i){
		if (fd == m_serverConfig.at(i).fd)
			return true;
	}
	return false;
}

void Server::run( void ){
	while (true){
		if (poll(m_sockets.data(), m_sockets.size(), TIMEOUT) == -1){
			throw std::runtime_error(SYS_ERROR("poll"));
		}
		for (unsigned long i = 0; i < m_sockets.size(); ++i){
			if (isServerSocket(m_sockets.at(i).fd)){
				addConnection(m_sockets.at(i).fd);
			}
			else if (m_sockets.at(i).revents & POLLIN){
				handleRequest(m_sockets.at(i));
				m_sockets.at(i).revents = 0;
				m_sockets.at(i).events = 0;

			}
		}
	}
}

void Server::addConnection( int serverFD ){
	pollfd newClient;
	struct sockaddr_in newClientAddr;
	socklen_t addrlen = sizeof(newClientAddr);

	do {
		std::memset(&newClient, 0, sizeof(newClient));
		errno = 0;
		newClient.fd = accept(serverFD, NULL, NULL);
		if (newClient.fd == -1){ // muss hier Errno geprüft werden?
			return ;
		}
		newClient.events = POLLIN | POLLOUT;
		m_sockets.push_back(newClient);
		m_clientServerMap[newClient.fd] = serverFD;
		if (getsockname(newClient.fd, (struct sockaddr *)&newClientAddr, &addrlen) == -1) throw std::runtime_error(SYS_ERROR("getsockname"));
		m_socketsIP[newClient.fd] = convertIPtoString(newClientAddr.sin_addr.s_addr);
	} while (newClient.fd != -1);

}

t_server Server::findConfig(pollfd client){
	int serverFD = m_clientServerMap[client.fd];
	if (!serverFD)
		return *m_serverConfig.begin();
	for (std::vector<t_server>::iterator it = m_serverConfig.begin(); it != m_serverConfig.end(); ++it){
		if (it->fd == serverFD)
			return *it;
	}
	return *m_serverConfig.begin();
}

void Server::removeClient( pollfd client ){
	m_socketsIP.erase(client.fd);
	m_clientServerMap.erase(client.fd);
	// loop needed, since comparison between pollfd is invalid
	for (std::vector<pollfd>::iterator it = m_sockets.begin(); it != m_sockets.end(); ++it){
		if (it->fd == client.fd){
			m_sockets.erase(it);
			break;
		}
	}
}

void Server::handleRequest( pollfd client ){
	Request request(client, findConfig(client));

/*
	if (err == 0){ // socket now closed -> erase socket

		}
		return ;
	}
	else if (err == -1){
		//hier muss evtl. errno geprüft werden
		return ;
	}
	try {
		Response response(request, m_serv, m_serv.locations[request.getLocationName()], m_socketsIP[client.fd]);
		if (send(client.fd, response.returnResponse(), response.getSize(), 0) == -1) throw std::runtime_error(SYS_ERROR("send"));
	}	
	catch(const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
*/
}

Server::~Server() {
	for (std::vector<t_server>::iterator it = m_serverConfig.begin(); it != m_serverConfig.end(); ++it){
		close(it->fd);
	}
}