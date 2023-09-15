
# include "Server.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Error.hpp"
# include "Config.hpp"

/* ###################################################### */

/* ###################################################### */

Server::Server( Config config ): m_serv(config.getServerConfig().at(0)){

	std::memset(&m_serverSocket, 0, sizeof(m_serverSocket));

	m_serverSocket.events = POLLIN | POLLOUT;
	m_serverSocket.fd = (socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0));
	if (m_serverSocket.fd == -1){
		throw std::runtime_error(SYS_ERROR("socket"));
	}

	int on = 1;
	if (setsockopt(m_serverSocket.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
		close(m_serverSocket.fd);
		throw std::runtime_error(SYS_ERROR("setsockopt"));
	}

	sockaddr_in serverAddress;
	std::memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(m_serv.port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_serverSocket.fd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1){
		close(m_serverSocket.fd);
		throw std::runtime_error(SYS_ERROR("bind"));
	}
	if (listen(m_serverSocket.fd, CLIENT_MAX)){
		close(m_serverSocket.fd);
		throw std::runtime_error(SYS_ERROR("listen"));
	}
	m_sockets.push_back(m_serverSocket);
}

void Server::run( void ){
	while (true){
		if (poll(m_sockets.data(), m_sockets.size(), -1) == -1){
			throw std::runtime_error(SYS_ERROR("poll"));
		}
		for (unsigned long i = 0; i < m_sockets.size(); i++){
			if (m_sockets.at(i).fd == m_serverSocket.fd){
				addConnection();
			}
			else {
				respond(i);
			}
		}
	}
}

void Server::addConnection( void ){
	pollfd newClient;

	do {
		std::memset(&newClient, 0, sizeof(newClient));
		errno = 0;
		newClient.fd = accept(m_serverSocket.fd, NULL, NULL);
		if (newClient.fd == -1){ // muss hier Errno geprüft werden?
			return ;
		}
		newClient.events = POLLIN | POLLOUT;
		m_sockets.push_back(newClient);
	} while (newClient.fd != -1);

}


void Server::respond( int clientIndex ){
	int err;
	char buffer[HTTP_HEADER_LIMIT + m_serv.clientMaxBodySize];

	err = recv(m_sockets[clientIndex].fd, buffer, sizeof(buffer), 0);
	std::cout << buffer << std::endl;
	if (err == 0){ // socket now closed -> erase socket
		m_sockets.erase(m_sockets.begin() + clientIndex);
		return ;
	}
	if (err == -1){
		//hier muss evtl. errno geprüft werden
		return ;
	}
	Request currRequest(buffer, m_serv);

	try{
		Response currResponse(currRequest, m_serv, m_serv.locations[currRequest.getLocationName()]);
		send(m_sockets.at(clientIndex).fd, currResponse.getResponse(), currResponse.getSize(), 0);
	}	
	catch(const std::exception& e){
		std::cout << "woopsi\n"; // send(); <-- 400 Bad Request
	}
}