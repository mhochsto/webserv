
# include "Server.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Error.hpp"
# include "Config.hpp"

void Server::CreateServerSocket( t_config& server ){
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
	serverAddress.sin_port = htons(std::atoi(server.Data["listen"].begin()->c_str()));
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

Server::Server( std::vector<t_config> serverConfig): m_serverConfig(serverConfig){
	for (std::vector<t_config>::iterator it = m_serverConfig.begin(); it != m_serverConfig.end(); ++it){
		print(Notification, "starting Server: " + *it->Data["hostname"].begin());
		CreateServerSocket(*it);
		print(Notification, "Server started Successfully: " + *it->Data["hostname"].begin());
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
				if (m_sockets.at(i).revents & POLLIN){
					addConnection(m_sockets.at(i).fd);
				}
			}
			else if (m_sockets.at(i).revents != 0) {
				if (m_sockets.at(i).revents & POLLIN) {
					if (handleRequest(m_clients[m_sockets.at(i).fd])){
						//m_sockets.at(i).revents = 0;
					}
				}
		   }
		}
	}
}

void Server::addConnection( int serverFD ){
	t_client newClient;
	pollfd newClientPoll;
	struct sockaddr_in newClientAddr;
	socklen_t addrlen = sizeof(newClientAddr);
	std::memset(&newClientPoll, 0, sizeof(newClientPoll));

	do {
		errno = 0;
		newClientPoll.fd = accept(serverFD, NULL, NULL);
		if (newClientPoll.fd == -1){
			return ;
		}
		newClientPoll.events = POLLIN | POLLOUT;
		m_sockets.push_back(newClientPoll);
		
		if (getsockname(newClientPoll.fd, (struct sockaddr *)&newClientAddr, &addrlen) == -1) throw std::runtime_error(SYS_ERROR("getsockname"));

		newClient.fd = newClientPoll.fd;
		newClient.serverFD = serverFD;
		newClient.ip = convertIPtoString(newClientAddr.sin_addr.s_addr);
		setConfig(newClient);
		m_clients[newClient.fd] = newClient;
		newClient.chunkSizeLong = -1;
		print(Notification, "new Client added (ip): " + newClient.ip);
	} while (newClient.fd != -1);

}

void Server::setConfig(t_client& client){
	for (std::vector<t_config>::iterator it = m_serverConfig.begin(); it != m_serverConfig.end(); ++it){
		if (it->fd == client.serverFD)
			client.config = *it;
	}
}

void Server::removeClient( t_client& client ){
	if (m_clients.find(client.fd) == m_clients.end()){
		return ; 
	}
	close (client.fd);
	m_clients.erase(client.fd);
}

chunkStatus	Server::recvChunks(t_client& client){
	char buffer[std::atoi(client.config.Data["client_max_body_size"].begin()->c_str())];
	std::memset(buffer, 0, sizeof(buffer));
	ssize_t rd = recv(client.fd, buffer, sizeof(buffer), 0);
	if (rd == 0){
		return BadRequest;
	}
	else if (rd == -1){
		return recvError;
	}
	else if (client.chunkSizeLong == -1){
		client.chunk += buffer;
		if (client.chunk.find("\r\n\r\n") == std::string::npos){
			return ChunkRecieved;
		}
		std::string chunkSizeAsString = client.chunk.substr(0, client.chunk.find("\r\n\r\n"));	
		char *end = NULL;
		client.chunkSizeLong = std::strtol(chunkSizeAsString.c_str(), &end, 16);
		if (*end){
			return BadRequest;
		}
	}

	client.chunk.erase(0, client.chunk.find("\r\n") + 2);
	if (client.chunkSizeLong == 0){
		return Complete;
	}
	if (client.chunk.length() != (unsigned long)client.chunkSizeLong){
		return BadRequest;
	}
	client.body += client.chunk;
	client.chunk.clear();
	client.chunkSizeLong = -1;
	return ChunkRecieved;
}

ssize_t Server::recvHeader(t_client& client){
	char c_buffer[HTTP_HEADER_LIMIT + std::atoi(client.config.Data["client_max_body_size"].begin()->c_str())];
	std::memset(c_buffer, 0, sizeof(c_buffer));
	ssize_t readBytes = recv(client.fd, c_buffer, sizeof(c_buffer), 0);
	if (readBytes <= 0){
		return readBytes;
	}
	client.header += c_buffer;
	return readBytes;
}

bool Server::headerFullyRecieved(t_client& client){
	return (client.header.find("\r\n\r\n") != std::string::npos);
}

void Server::sendResponse( t_client& client, std::string status ){
	print(Notification, "recieved request from " + client.ip);
	print(Notification, client.header.substr(0, client.header.find("\n")));

	if (status != "Ok"){
		client.header = status;
	}
	Request request(client);

	Response response(client, request);
	send(client.fd, response.returnResponse(), response.getSize(), 0);
	
	std::string respStr(response.returnResponse());
	print(Notification, "Response sent to: " + client.ip);
	print(Notification, respStr.substr(0, respStr.find("\n")));
	
	client.header.clear();
	client.body.clear();
	client.chunk.clear();
	client.chunkSizeLong = -1;
}

int Server::handleRequest( t_client& client ){
	ssize_t recvValue;
	if (!headerFullyRecieved(client)){
		recvValue = recvHeader(client);
		if (recvValue == 0){
			return 1;
		}
		if (recvValue == - 1){
			removeClient(client);
			return 1;
		}
		if (!headerFullyRecieved(client)){
			return 1;
		}
		if (client.header.find("Transfer-Encoding: chunked\r\n") == std::string::npos){
			client.body = client.header.substr(client.header.find("\r\n\r\n") + 4);
			sendResponse(client, "Ok");
			return 0;
		}
	}

	/* this part will only be reached if the request is chunked */
	switch (recvChunks(client)){
		case BadRequest:
			sendResponse(client, "BadRequest");
			return 0;
		case ChunkRecieved:
			return 1;
		case Complete:
			sendResponse(client, "Ok");
			return 0;
		case recvError:
			removeClient(client);
			return 1;
	}
}

Server::~Server() {
	for (std::vector<t_config>::iterator it = m_serverConfig.begin(); it != m_serverConfig.end(); ++it){
		close(it->fd);
	}
}