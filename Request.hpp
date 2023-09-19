

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include<string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <map>

#include "Config.hpp"

class Request {
    private:
        std::string                         m_requestType;
        std::string                         m_requestPath;
        std::string                         m_requestHttpVersion;
        std::string                         m_requestBody;
        std::map<std::string, std::string>  m_requestHeader;
        t_server                            m_serv;

        std::map<std::string, std::string>  getRequestHeader( void ) const;
    public:
        Request( std::string rawRequest, t_server serv);
        ~Request();

        std::string                         getType( void ) const;
        std::string                         getPath( void ) const;
        std::string                         getHttpVersion( void ) const;
        std::string                         getBody( void ) const;
        std::string                         getLocationName( void );
};


#endif