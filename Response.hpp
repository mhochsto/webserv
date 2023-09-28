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
        t_client    m_client;
        int         m_responseSize;
        std::map<std::string, std::string> m_UrlParameter; 
        typedef void (Response::*funcPtr)(Request&);
        
        std::map<std::string, funcPtr> m_responseMap;
        void getResponse( Request& request );
        void postResponse( Request& request );
        void deleteResponse( Request& request );
        void putResponse( Request& request );

        void saveGetParam( std::string content ); 
        std::string showDir(std::string path);
        void createResponse(std::string rspType, std::string file);
        void cgiResponse( std::string path, Request& request, std::string rawUrlParameter );
        void create404Response(void);
    public:
        Response( t_client& client, Request& request );
        const char *returnResponse( void );
        int  getSize( void );

};


std::string timestamp(void);
# define FOF_PATH "./website/pages/404.html"




#endif