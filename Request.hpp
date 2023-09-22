

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

#include "Config.hpp"
#include "Error.hpp"
#include "utils.hpp"

# define HTTP_HEADER_LIMIT 8192

class Request {
    private:
        std::string m_requestLine;


        std::string                         m_requestType;
        std::string                         m_requestPath;
        std::string                         m_requestHttpVersion;
        std::string                         m_requestBody;
        std::map<std::string, std::string>  m_requestData;
        std::map<std::string, std::string>  m_requestHeader;
        t_server                            m_serv;
        std::string                         m_client;

        std::map<std::string, std::string>  getRequestHeader( void ) const;
    public:
        /* FD of current Client Socket, Server, Client IP*/
        Request(t_server serv);
        ~Request();
        int     receive(int clientFD);

        std::string                         getType( void ) const;
        std::string                         getPath( void ) const;
        std::string                         getHttpVersion( void ) const;
        std::map<std::string, std::string>  getData( void ) const;
        std::string                         getBody( void ) const;
        std::string                         getLocationName( void );
        std::string                         getRequestHttpVersion( void ) const;
        int                                 validateRequestLine( void );
        std::string                         get( std::string str ) const;
        bool                                requestContains( std::string str ) const;
        void                                setPath( std::string newPath);
};


#endif