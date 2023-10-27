#ifndef SERVER_HPP
# define SERVER_HPP

# include <iomanip>
# include <iostream>
# include <exception>
# include <sys/socket.h> //socket
# include <unistd.h> // close

# define BUFFER_SIZE 12000;

// maybe let the constructor take ip_address and port

class Server {
	public:
		//Con- and Destuctors
		Server();
		Server(const Server &src);
		~Server();

		//Operator Overloads
		const Server	&operator=(const Server &rhs);

		//Memberfunctions
		void			run();
	private:
		//Member Variables
		struct sockaddr_in	socketAddress;
		std::string			IPAdress;
		int					socketFD;
		int					port;

};

#endif