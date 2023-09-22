
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
		if (poll(m_sockets.data(), m_sockets.size(), TIMEOUT) == -1){
			throw std::runtime_error(SYS_ERROR("poll"));
		}
		for (unsigned long i = 0; i < m_sockets.size(); i++){
			if (m_sockets.at(i).fd == m_serverSocket.fd){
				addConnection();
			}
			else {
				respond(m_sockets.at(i));
			}
		}
	}
}

std::string Server::convertIPtoString(unsigned long ip){
    std::stringstream sstream;
	sstream << (int)((ip >> 24) & 0xFF);
	sstream << ".";
    sstream << (int)((ip >> 16) & 0xFF);
	sstream << ".";
    sstream << (int)((ip >> 8) & 0xFF);
	sstream << ".";
    sstream << (int)(ip & 0xFF);
	return sstream.str();
}

void Server::addConnection( void ){
	pollfd newClient;
	struct sockaddr_in newClientAddr;
	socklen_t addrlen = sizeof(newClientAddr);

	do {
		std::memset(&newClient, 0, sizeof(newClient));
		errno = 0;
		newClient.fd = accept(m_serverSocket.fd, NULL, NULL);
		if (newClient.fd == -1){ // muss hier Errno geprüft werden?
			return ;
		}
		newClient.events = POLLIN | POLLOUT;
		m_sockets.push_back(newClient);
		if (getsockname(newClient.fd, (struct sockaddr *)&newClientAddr, &addrlen) == -1) throw std::runtime_error(SYS_ERROR("getsockname"));
		m_socketsIP[newClient.fd] = convertIPtoString(newClientAddr.sin_addr.s_addr);
	} while (newClient.fd != -1);

}


void Server::respond( pollfd client ){

	Request request(m_serv);
	
	int err = request.receive(client.fd);
	if (err == 0){ // socket now closed -> erase socket
		m_socketsIP.erase(client.fd);
		/* loop needed, since comparison between pollfd is invalid */
		for (std::vector<pollfd>::iterator it = m_sockets.begin(); it != m_sockets.end(); ++it){
			if (it->fd == client.fd){
				m_sockets.erase(it);
				break;
			}
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
}