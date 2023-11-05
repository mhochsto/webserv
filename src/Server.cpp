
# include "Server.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Error.hpp"
# include "Config.hpp"
# include "CgiHandler.hpp"

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

	int s = getaddrinfo(server.serverIP.c_str(), port.str().c_str(), &hints, &results);
	if (s){
		throw std::runtime_error(gai_strerror(s));
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
		perror("bind");
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
		print(Notification, "starting Server: " + it->serverName);
		try{
			CreateServerSocket(*it);
		}
		catch(const std::exception& e){
			std::cerr << e.what() << '\n';
			continue ;
		}
		print(Notification, "Server started Successfully: " + it->serverName + "\n");
	}
}

bool Server::isServerSocket(int fd){
	for(unsigned long i = 0; i < m_serverConfig.size(); ++i){
		if (fd == m_serverConfig.at(i).fd)
			return true;
	}
	return false;
}

bool	activeCGI(t_client& client){
	if (!client.request->getIsCgi()){
		return false;
	}
	if (client.activeCGI){
		return true ;
	}
	if (!client.activeCGI && !client.cgi){
		client.cgi = new CgiHandler(client);
		client.activeCGI = true;
		return true;
	}

	return false;
}

t_client* Server::findClient(int pipeFd){
	for (std::map<int, t_client>::iterator it = m_clients.begin(); it != m_clients.end(); ++it){
		if (it->second.cgi){
			if (it->second.cgi->isPipeFd(pipeFd)){
				return &it->second;
			}
		}
	}
	return NULL;
}


void Server::handleClient(pollfd& client){
	if (m_clients[client.fd].lastAction + REQUEST_TIMEOUT < time(NULL) && !m_clients[client.fd].header.empty()){
		t_client *currentClient = &m_clients[client.fd];
		if (currentClient->activeCGI){
			removeCgiSockets(currentClient);
		}
		currentClient->request = new Request(*currentClient, m_sockets);
		currentClient->request->setInvalidRequest("408 Request Timeout\n");
		sendResponse(m_clients[client.fd]);
		return ;
	}
	if (client.revents & POLLIN) {
		recieveRequest(m_clients[client.fd]);
	}
	else if (client.revents & POLLOUT && m_clients[client.fd].recieving == done){
		if (activeCGI(m_clients[client.fd])){
			return ;
		}
		sendResponse(m_clients[client.fd]);
	}
	else if (client.revents & (POLLHUP | POLLERR)){
		removeClient(m_clients[client.fd]);
	}
}

void Server::removeCgiSockets(t_client *cgiClient){
	int in, out;
	cgiClient->cgi->CgiSocketsToRemove(&in, &out);
	removeFdFromSocketVec(in);
	removeFdFromSocketVec(out);
	kill(cgiClient->CgiPid, SIGKILL);
}

void Server::handleCgiSockets(pollfd& pollfd){
	t_client *cgiClient = findClient(pollfd.fd);
	if (cgiClient->lastAction + CGI_TIMEOUT < time(NULL)){
		removeCgiSockets(cgiClient);
		cgiClient->request->setInvalidRequest("408 Request Timeout\n");
		sendResponse(*cgiClient);
		return ;
	}

  	if (pollfd.revents & POLLIN){
		int wt = write(pollfd.fd, cgiClient->body.c_str(), cgiClient->body.length());
		cgiClient->body.erase(0, wt);
		if (wt <= 0 || cgiClient->body.size() == 0){
			removeFdFromSocketVec(pollfd.fd);
			return ;
		}
	}
	else if (pollfd.revents & POLLOUT){
		char buf[1024];
		std::memset(buf, 0, 1024);
		int rd = read(pollfd.fd, buf, 1024);
		if (rd == -1){ //  rd == -1 happens if Script is not done with execution yet. 
			return;
		}
		if (rd == 0){
			removeCgiSockets(cgiClient);
			cgiClient->body = cgiClient->cgi->getOutput();
			cgiClient->activeCGI = false;
			return ;
		}
		cgiClient->cgi->getOutput() += buf;
	}
	pollfd.revents = 0;
}

bool Server::m_stop = true;

void		Server::sigIntHandler(int signal ){
	(void)signal;
	m_stop = false;
}

void Server::run( void ){
	if (m_sockets.size() == 0){
		throw std::runtime_error("Server::No open Server Sockets");
	}

	signal(SIGINT, &Server::sigIntHandler);
	
	while (true){
		if (poll(m_sockets.data(), m_sockets.size(), -42) == -1){
			if (!m_stop) {
				std::cout << "Stopping Webserv ... ";
				break ;
			}
			throw std::runtime_error(SYS_ERROR("poll"));
		}
		for (unsigned long i = 0; i < m_sockets.size(); ++i){
			if (isServerSocket(m_sockets.at(i).fd)){
				if (m_sockets.at(i).revents & POLLIN){
					addConnection(m_sockets.at(i).fd);
					m_sockets.at(i).revents = 0;
				}
			}
			else if (isClientSocket(m_sockets.at(i))) {
				handleClient(m_sockets.at(i));
		   }
		   else if (findClient(m_sockets.at(i).fd)) {
				handleCgiSockets(m_sockets.at(i));
		   }
		}
	}
}

void Server::addConnection( int serverFD ){
	t_client newClient;
	pollfd newClientPoll;
	struct sockaddr_in newClientAddr;
	socklen_t addrlen = sizeof(newClientAddr);
	std::memset(&newClientAddr, 0, sizeof(newClientAddr));
	std::memset(&newClientPoll, 0, sizeof(newClientPoll));

	newClientPoll.fd = accept(serverFD, NULL, NULL);
	if (newClientPoll.fd == -1){
		return ;
	}
	newClientPoll.events = POLLIN | POLLOUT | POLLHUP; 
	
	if (getsockname(newClientPoll.fd, (struct sockaddr *)&newClientAddr, &addrlen) == -1){
		close(newClientPoll.fd);
		return ;
	}
	m_sockets.push_back(newClientPoll);
	newClient.fd = newClientPoll.fd;
	newClient.serverFD = serverFD;
	newClient.ip = convertIPtoString(newClientAddr.sin_addr.s_addr);
	newClient.config = setConfig(serverFD);
	newClient.recieving = header;
	newClient.lastAction = time(NULL);
	newClient.socketVector = &m_sockets;
	m_clients[newClient.fd] = newClient;
	print(Notification, "new Client added (ip): " + newClient.ip);
}

t_config& Server::setConfig(int serverFD){
	for (std::vector<t_config>::iterator it = m_serverConfig.begin(); it != m_serverConfig.end(); ++it){
		if (it->fd == serverFD)
			return *it;
	}
	return *m_serverConfig.begin();
}

void Server::removeFdFromSocketVec( int fd ){
	for (std::vector<pollfd>::iterator it = m_sockets.begin(); it != m_sockets.end(); ++it){
		if (it->fd == fd){
			close(fd);
			m_sockets.erase(it);
			break ;
		}
	}
}

void Server::removeClient( t_client& client ){
	print(Notification, "Removed Client: " + client.ip);
	if (m_clients.find(client.fd) == m_clients.end()){
		return ; 
	}
	if (client.request){
		delete client.request;
		client.request = NULL;
	}
	if (client.cgi){
		delete client.cgi;
		client.cgi = NULL;
	}
	removeFdFromSocketVec(client.fd);
	m_clients.erase(client.fd);
}

ssize_t Server::recvFromClient(std::string&data, t_client& client){
	char c_buffer[HTTP_HEADER_LIMIT];
	std::memset(c_buffer, 0, sizeof(c_buffer));
	ssize_t readBytes = recv(client.fd, c_buffer, sizeof(c_buffer), 0);
	if (readBytes <= 0){
		removeClient(client);
		return 0;
	}
	data += c_buffer;
	return readBytes;
}

void Server::sendResponse( t_client& client){
	print(Notification, "recieved request from " + client.ip);
	print(Notification, client.header.substr(0, client.header.find("\n")));


	Response response(client);
	send(client.fd, response.returnResponse(), response.getSize(), 0);
	
	std::string respStr(response.returnResponse());
	print(Notification, "Response sent to: " + client.ip);
	print(Notification, respStr.substr(0, respStr.find("\n")));
	
	client.header.clear();
	client.body.clear();
	client.chunk.clear();
	client.chunkSizeLong = -1;
	client.recieving = header;
	client.activeCGI = false;
	client.exptectedBodySize = 0;
	client.CgiPid = 0;
	if (client.request){
		delete client.request;
		client.request = NULL;
	}
	if (client.cgi){
		delete client.cgi;
		client.cgi = NULL;
	}
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
		std::string contentLength = client.header.substr(client.header.find("Content-Length:") + std::strlen("Content-Length:"));
		contentLength.resize(contentLength.find_first_of('\r'));
		if (contentLength.find_first_of("0123456789") == std::string::npos){
			client.exptectedBodySize = 0;
			return ;
		}
		client.exptectedBodySize = std::strtol(contentLength.c_str(), NULL, 10);
		if (client.exptectedBodySize == (long)client.body.size()){
			client.recieving = done;
		}
	}
	else {
		client.recieving = done;
	}
	client.header.erase(client.header.find("\r\n\r\n"));
}

int Server::setChunkSize( t_client& client ){
	if (client.chunkSizeLong != -1){
		return 0;
	}
	if (client.chunk.find("\r\n") == std::string::npos || client.chunk.size() <= 2){
		return 1;
	}
	char *end = NULL;
	std::string chunkSizeAsString = client.chunk.substr(0, client.chunk.find("\r\n"));	
	client.chunkSizeLong = std::strtol(chunkSizeAsString.c_str(), &end, 16);
	client.chunk.erase(0, client.chunk.find("\r\n") + 2);
	return 0;
}

/* +2 for "\r\n" */
void Server::saveChunk(t_client& client){
	if (client.chunkSizeLong == 0 ){
		if (client.chunk == "\r\n" || client.chunk == "0\r\n\r\n"){
			client.recieving = done;
		}
		return ;
	}

	if ((long)client.chunk.size() < client.chunkSizeLong + 2){
		return ;
	}
	else if ((long)client.chunk.size() > client.chunkSizeLong + 2){

		client.body += client.chunk.substr(0, client.chunkSizeLong);
		client.chunk.erase(0, client.chunkSizeLong + 2);
		client.chunkSizeLong = -1;
		setChunkSize(client);
		if (client.chunkSizeLong == 0 ){
			if (client.chunk == "\r\n" || client.chunk == "0\r\n\r\n"){
				client.recieving = done;
			}
			return ;
		}
	}
	else if (client.chunk.find("\r\n") == client.chunk.size() - 2){
		client.body += client.chunk.substr(0, client.chunkSizeLong);
		client.chunk.clear();
		client.chunkSizeLong = -1;
	}
}

void Server::recieveRequest( t_client& client ){
	client.lastAction = time(NULL);
	switch (client.recieving){
		case header:
			if (recvFromClient(client.header, client) <= 0){
				return ;	
			}
			setRecieveState(client);
			break;
		case body:
			if (recvFromClient(client.body, client) <= 0){
				return ;
			}
			if ((long)client.body.size() == client.exptectedBodySize){
				client.recieving = done;
				break ;
			}
			return ;
		case chunk:
			if (recvFromClient(client.chunk, client) <= 0){
				return ;
			}
			if (!setChunkSize(client)){
				saveChunk(client);
			}
			break ;
		default:
			break;
		}
		if (client.recieving == done && !client.request){
				client.request = new Request(client, m_sockets);
		}
}

Server::~Server() {
	for (std::map<int, t_client>::iterator it = m_clients.begin(); it != m_clients.end(); ++it){
		if (it->second.cgi){
			if (it->second.activeCGI){
				kill(it->second.CgiPid, SIGKILL);
			}
			delete it->second.cgi;
			it->second.cgi = NULL;
		}
		if (it->second.request){
			delete it->second.request;
			it->second.request = NULL;
		}
	} 
	for (std::vector<pollfd>::iterator it = m_sockets.begin(); it != m_sockets.end(); ++it){
		close(it->fd);
	}
}