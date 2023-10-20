
# include "Server.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Error.hpp"
# include "Config.hpp"


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void Server::CreateServerSocket( t_config& server ){
	pollfd	serverSocket;
	struct addrinfo hints;
	struct addrinfo *results;

	std::memset(&serverSocket, 0, sizeof(serverSocket));
	std::memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	
	std::stringstream port;
	port << server.port;
	int s = getaddrinfo(server.serverName.c_str(), port.str().c_str(), &hints, &results);
	if (s){
		std::cerr << gai_strerror(s) << std::endl;
	}
	serverSocket.events = POLLIN | POLLOUT;
	serverSocket.fd = socket(results->ai_family, results->ai_socktype | SOCK_NONBLOCK, results->ai_protocol);
	if (serverSocket.fd == -1){
		freeaddrinfo(results);
		throw std::runtime_error(SYS_ERROR("socket"));
	}

	int on = 1;
	if (setsockopt(serverSocket.fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
		close(serverSocket.fd);
		freeaddrinfo(results);
		throw std::runtime_error(SYS_ERROR("setsockopt"));
	}
	if (bind(serverSocket.fd, results->ai_addr, results->ai_addrlen) == -1){
		close(serverSocket.fd);
		freeaddrinfo(results);
		throw std::runtime_error(SYS_ERROR("bind"));
	}
	freeaddrinfo(results);
	if (listen(serverSocket.fd, CLIENT_MAX)){
		close(serverSocket.fd);
		throw std::runtime_error(SYS_ERROR("listen"));
	}
	m_sockets.push_back(serverSocket);
	server.fd = serverSocket.fd;
}

Server::Server( std::vector<t_config> serverConfig): m_serverConfig(serverConfig){
	for (std::vector<t_config>::iterator it = m_serverConfig.begin(); it != m_serverConfig.end(); ++it){
		print(Notification, "starting Server");
		CreateServerSocket(*it);
		print(Notification, "Server started Successfully");
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
					m_sockets.at(i).revents = 0;
				}
			}
			else if (m_sockets.at(i).revents != 0) {
				if (m_sockets.at(i).revents & POLLIN) {
					handleRequest(m_clients[m_sockets.at(i).fd]);
					m_sockets.at(i).revents = 0;
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

	newClientPoll.fd = accept(serverFD, NULL, NULL);
	if (newClientPoll.fd == -1){
		return ;
	}
	newClientPoll.events = POLLIN | POLLOUT;
	
	if (getsockname(newClientPoll.fd, (struct sockaddr *)&newClientAddr, &addrlen) == -1){
		close(newClientPoll.fd);
		return ;
	}
	m_sockets.push_back(newClientPoll);
	newClient.fd = newClientPoll.fd;
	newClient.serverFD = serverFD;
	newClient.ip = convertIPtoString(newClientAddr.sin_addr.s_addr);
	setConfig(newClient);
	newClient.chunkSizeLong = -1;
	newClient.recieving = header;
	m_clients[newClient.fd] = newClient;
	print(Notification, "new Client added (ip): " + newClient.ip);
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

void Server::setChunkSize( t_client& client ){
	if (client.chunkSizeLong != -1){
		return ;
	}
	if (client.chunk.find("\r\n") == std::string::npos){
		return ;
	}
	char *end = NULL;
	std::string chunkSizeAsString = client.chunk.substr(0, client.chunk.find("\r\n"));	
	std::cerr << "chunk size: " << chunkSizeAsString << std::endl;
	client.chunkSizeLong = std::strtol(chunkSizeAsString.c_str(), &end, 16);
	client.chunk.erase(0, client.chunk.find("\r\n") + 2);
	if (client.chunkSizeLong == 0 && client.chunk == "\r\n"){
		client.recieving = done;
	}
}




ssize_t Server::recvFromClient(std::string&data, t_client& client){
	char c_buffer[HTTP_HEADER_LIMIT + client.location.clientMaxBodySize];
	std::memset(c_buffer, 0, sizeof(c_buffer));
	ssize_t readBytes = recv(client.fd, c_buffer, sizeof(c_buffer), 0);
	std::cout << "["<<c_buffer<<"]";
	sleep(0.03);
	if (readBytes == 0){
		removeClient(client);
		return 0;
	}
	if (readBytes  == -1){
		removeClient(client);
		return -1;
	}
	data += c_buffer;
	return readBytes;
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
	client.recieving = header;
}

int	Server::recieveData(std::string& data, t_client& client){
	if (client.location.clientMaxBodySize == -1){
		client.location.clientMaxBodySize = client.config.clientMaxBodySize;
	}
	if (data.find("\r\n\r\n") != std::string::npos){
		return 0;
	}
	if (recvFromClient(data, client) <= 0){
		return 1;
	}
	if (data.find("\r\n\r\n") != std::string::npos){
		return 0;
	}
	return 1;
}


void Server::setRecieveState(t_client& client){
	if (client.header.find("\r\n\r\n") == std::string::npos){
		return ;
	}
	if (client.header.find("Transfer-Encoding: chunked\r\n") != std::string::npos){
		client.recieving = chunk;
		client.chunk = client.header.substr(client.header.find("\r\n\r\n") + 4);
	}
	else if (client.header.find("Content-Length:") != std::string::npos){
		client.recieving = body;
		client.body = client.header.substr(client.header.find("\r\n\r\n") + 4);
		if (client.header.find_first_of("0123456789") == std::string::npos){
			client.exptectedBodySize = 0;
			return ;
		}
		client.exptectedBodySize = std::strtol(client.header.substr(client.header.find_first_of("0123456789")).c_str(), NULL, 10);
	}
	else {
		client.recieving = done;
	}
	client.header.erase(client.header.find("\r\n\r\n"));
}

void Server::saveChunk(t_client& client){
	if ((long)client.chunk.size() < client.chunkSizeLong){
		return ;
	}
	else if ((long)client.chunk.size() > client.chunkSizeLong){
		client.body += client.chunk.substr(0, client.chunkSizeLong);
		client.chunk.erase(0, client.chunkSizeLong);
		setChunkSize(client);
	}
	else {
		client.body += client.chunk;
		client.chunk.clear();
	}
}

void Server::handleRequest( t_client& client ){

	switch (client.recieving)
	{
	case header:
		if (recvFromClient(client.header, client) <= 0){
			return ;	
		}
		setRecieveState(client);
		break ;
	case body:
		if (recvFromClient(client.body, client) <= 0){
			return ;
		}
		if ((long)client.body.size() == client.exptectedBodySize){
			client.recieving = done;
		}
		return ;
	case chunk:
		if (recvFromClient(client.chunk, client) <= 0){
			return ;
		}
		setChunkSize(client);
		saveChunk(client);
		break ;
	default:
		break;
	}
	if (client.recieving == done){
		sendResponse(client, "Ok");
	}
/*
	if (recieveData(client.header, client)){
		return ;
	}
	if (client.header.find("Transfer-Encoding: chunked\r\n") == std::string::npos){
		if (!std::strncmp(client.header.c_str(), "POST", std::strlen("POST")) && recieveData(client.body, client)){
			return ;
		}
		client.header.erase(client.header.find("\r\n\r\n"));
		sendResponse(client, "Ok");
		return ;
	}
	if (client.body.empty()){
		client.body = client.header.substr(client.header.find("\r\n\r\n") + 4);
		client.chunk += client.body;
	}
	switch (recvChunks(client)){
		case BadRequest:
			sendResponse(client, "BadRequest");
			return ;
		case ChunkRecieved:
			return ;
		case Complete:
			client.header.erase(client.header.find("\r\n\r\n"));
			sendResponse(client, "Ok");
			return ;
		case recvError:
			removeClient(client);
			return ;
	}
*/
}

Server::~Server() {
	for (std::vector<t_config>::iterator it = m_serverConfig.begin(); it != m_serverConfig.end(); ++it){
		close(it->fd);
	}
}