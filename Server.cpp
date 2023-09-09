
#include "Server.hpp"

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
				handleRequest(i);
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

void Server::constructRequest(char buffer[], std::stringstream &response){
	
	std::stringstream buf(&buffer[0]);
	std::stringstream lineStream;
	std::string line;
	std::string word;
	std::vector<std::string> requestLine;
	std::stringstream respose;

	respose << "";
	std::getline(buf, line);
	lineStream << line;
	do {
		getline(lineStream, word, ' ');
		if (std::find(requestLine.begin(), requestLine.end(), word) == requestLine.end())
			requestLine.push_back(word);
		else
			break ;
	} while(true);
	if (requestLine.size() != 3){
		// 400
		return ;
	}
	if (!std::strcmp(requestLine.at(2).c_str(), "HTTP/1.1\r\n")){ //strcmp needed for \r
		//not HTTP/1.1
		return ;
	}
	/* mit Map ersetzten */
	std::string validate[] = VALID_REQUESTS;
	void (Server::*f[])(std::string, std::vector<std::string>, std::stringstream&) = REQUEST_FUNCTIONS;
	for (unsigned long i = 0; i < validate->size(); i++){
		if (validate[i] == requestLine.at(REQUEST)){
			((this->*f[i])(std::string(buffer), requestLine, response));
			return ;
		}
	}
	return ; // 400?

}

void Server::get(std::string buffer, std::vector<std::string> requestLine, std::stringstream &response){

	response << "HTTP/1.1";
	if (requestLine.at(PATH) == "/")
		requestLine.at(PATH) = "/index.html";
	requestLine.at(PATH) = "./website" + requestLine.at(PATH);

	if (access(requestLine.at(PATH).c_str(), F_OK) == -1){
		return ; //hier muss die 404 Seite zurückgegeben werden
	}
	
	struct stat statbuf;
	std::memset(&statbuf, 0 , sizeof(struct stat));
	if (stat(requestLine.at(PATH).c_str(), &statbuf) == -1) throw std::runtime_error(ERROR("stat"));
	if (!S_ISREG(statbuf.st_mode)){
		// not a regular file
		return ;
	} 
	response <<" 200 OK\nContent-Type: text/html\n"; // evtl. anpassen falls es andere files gibt
	std::stringstream length;
	length << statbuf.st_size;
	response << "Content-Length: " << length.str() << "\n\n";

	std::fstream file(requestLine.at(PATH).c_str());
	if (!file) throw std::runtime_error(ERROR("can't open source file"));
	response << file.rdbuf();
	(void)buffer;
}

void Server::del(std::string buffer, std::vector<std::string> requestLine, std::stringstream &response){
	(void)buffer;
	(void)requestLine;
	(void)response;
}

void Server::post(std::string buffer, std::vector<std::string> requestLine, std::stringstream &response){
	(void)buffer;
	(void)requestLine;
	(void)response;
}

void Server::handleRequest ( int clientIndex ){
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

	std::stringstream response;
	constructRequest(buffer, response);
	send(m_sockets.at(clientIndex).fd, response.str().c_str(), response.str().length(), 0);

}