

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include<string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <map>

#include <sys/socket.h>
#include <cstdlib>
#include <poll.h>

#include "Config.hpp"
#include "Error.hpp"
#include "utils.hpp"
# include "Server.hpp"

# define HTTP_HEADER_LIMIT 8192

class Request {
	private:
		t_server		m_config;
		t_client		m_client;
		std::string     m_requestType;
		std::string     m_requestPath;
		std::string     m_requestHttpVersion;
		std::map<std::string, std::string> m_requestData;
		std::string		m_requestBody;

		int parseHeader( std::string& header );
		int 	validateAndSetRequestLine( std::string line );
	public:
		Request(t_client client);
		~Request();
	
	bool        contains( std::string str ) const;
	std::string get( std::string str ) const;
	std::string getLocationName( void );
	std::string getType( void );
	std::string getPath( void );
	std::string getBody( void );
	std::string getClientIP( void );
	void		setPath( std::string newPath );
};


#endif