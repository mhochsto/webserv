

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <map>

#include <sys/socket.h>
#include <cstdlib>
#include <poll.h>

# include "DataStructs.hpp"
# include "utils.hpp"
# include "Error.hpp"

# define HTTP_HEADER_LIMIT 8192

class Request {
	private:
		t_config		m_config;
		t_client&		m_client;
		std::string     m_requestType;
		std::string     m_requestPath;
		std::string     m_requestHttpVersion;
		std::string		m_requestBody;
		std::map<std::string, std::string> m_requestData;

		int 		validateAndSetRequestLine( std::string line );
		std::string getLocationName( void );
	public:
		Request(t_client& client);
		~Request();
	
		bool        contains( std::string str ) const;
		std::string get( std::string str ) const;
		std::string getType( void );
		std::string getPath( void );
		std::string getBody( void );
		std::string getClientIP( void );
		void		setPath( std::string newPath );
};

#endif