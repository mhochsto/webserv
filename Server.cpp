
# include "Server.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Error.hpp"

Server::Server( void ){

	m_httpBodyLimit = 10000;
	//config
	std::memset(&m_serverSocket, 0, sizeof(m_serverSocket));

	m_serverSocket.fd = (socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0));
	m_serverSocket.events = POLLIN | POLLOUT;

	if (m_serverSocket.fd == -1){
		throw std::runtime_error(ERROR("socket"));
	}
	int on = 1;
	if (setsockopt(m_serverSocket.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
		close(m_serverSocket.fd);
		throw std::runtime_error(ERROR("setsockopt"));
	}

	sockaddr_in serverAddress;
	std::memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8080);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_serverSocket.fd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1){
		close(m_serverSocket.fd);
		throw std::runtime_error(ERROR("bind"));
	}

	if (listen(m_serverSocket.fd, 200)){
		close(m_serverSocket.fd);
		throw std::runtime_error(ERROR("listen"));
	}
	m_sockets.push_back(m_serverSocket);
}

void Server::run( void ){
	while (true){
		if (poll(m_sockets.data(), m_sockets.size(), -1) == -1){
			throw std::runtime_error(ERROR("poll"));
		}
		for (unsigned long i = 0; i < m_sockets.size(); i++){
			if (m_sockets.at(i).fd == m_serverSocket.fd){
				addConnection();
			}
			else{
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
	char buffer[HTTP_HEADER_LIMIT + m_httpBodyLimit];

	err = recv(m_sockets[clientIndex].fd, buffer, sizeof(buffer), 0);
	if (err == 0){
		m_sockets.erase(m_sockets.begin() + clientIndex);
		return ;
	}
	if (err == -1){ //hier muss evtl. errno geprüft werden
		return ;
	}
	Request currRequest(buffer);
	Response currResponse(currRequest);
	send(m_sockets.at(clientIndex).fd, currResponse.getResponse(), currResponse.getSize(), 0);

//	std::stringstream response;

}