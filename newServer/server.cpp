#include "server.hpp"

Server::Server() : IPAdress("127.0.0.1"), port(8080), socketAddressLen(sizeof(struct sockaddr_in)) {
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD < 0) {
		throw std::runtime_error("Server:: Cannot create the socket!");
	}
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(port);
	socketAddress.sin_addr.s_addr = inet_addr(IPAdress.c_str());
	if (bind(socketFD, (sockaddr *)&socketAddress, socketAddressLen) < 0) {
		throw std::runtime_error("Server:: Cannot bind to the socket!");
	}
	if (listen(socketFD, 20)) {
		throw std::runtime_error("Server:: Cannot listen to the socket!");
	}

}

Server::Server(const Server &src) {
	close(socketFD);
}

Server::~Server() {}

const Server	&Server::operator=(const Server &rhs) {}

void	Server::run() {
	while (true) {
		if (poll())
	}
}
