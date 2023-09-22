#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <algorithm>
# include <ctime>

#include <sys/stat.h> // stat()
#include <sys/types.h> // struct DIR; *dir()
#include <dirent.h> // struct dirent


#include <fcntl.h>

# include "Config.hpp"

class Request;

class Response {
    private:
        std::string m_response;
        std::string m_clientIP;
        int         m_responseSize;
        std::map<std::string, std::string> m_UrlParameter;
        t_location  m_location;
        t_server    m_serv;    
        typedef void (Response::*funcPtr)(Request&);
        
        std::map<std::string, funcPtr> m_responseMap;
        void getResponse( Request& request );
        void postResponse( Request& request );
        void deleteResponse( Request& request );

        void saveGetParam( std::string content ); 
        std::string showDir(std::string path);
        void createResponse(std::string rspType, std::string file);
        void cgiResponse( std::string path, Request& request, std::string rawUrlParameter );
    public:
        std::string getClientAddr( void );
        Response( Request& request, t_server serv, t_location location, std::string clientIP );
        const char *returnResponse( void );
        int  getSize( void );

};


std::string timestamp(void);
# define FOF_PATH "./website/pages/404.html"




#endif