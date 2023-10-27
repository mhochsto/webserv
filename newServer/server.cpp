#include "server.hpp"

Server::Server() : IPAdress("127.0.0.1"), port(8080){
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0)
        throw std::runtime_error("Server:: Cannot create the socket!");

}

Server::Server(const Server &src) {
    close(socketFD);
}

Server::~Server() {}

const Server    &Server::operator=(const Server &rhs) {}

void    Server::run() {}
