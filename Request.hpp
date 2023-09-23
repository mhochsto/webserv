

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

# define HTTP_HEADER_LIMIT 8192

class Request {
	private:
		ssize_t			m_readBytes;
		int				m_clientFD;
		t_server		m_config;
	
		std::string     m_requestType;
		std::string     m_requestPath;
		std::string     m_requestHttpVersion;
		std::map<std::string, std::string> m_requestData;
		std::string		m_requestBody;
int validateAndSetRequestLine( std::string line );

	public:
		Request(pollfd client, t_server config);
		~Request();
	
	bool        contains( std::string str ) const;
	std::string get( std::string str ) const;
	std::string getLocationName( void );
};


#endif