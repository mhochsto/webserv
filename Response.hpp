#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <map>
# include <string>
# include <utility>
# include <algorithm>
# include <fstream>
# include <ctime>

#include <unistd.h> // access()
#include <sys/stat.h> // stat()

# include "Config.hpp"

class Request;

class Response {
    private:
        std::string m_response;
        int m_responseSize;
        t_location m_location;
        t_server m_serv;    
        typedef void (Response::*funcPtr)(const Request&);
        
        std::map<std::string, funcPtr> m_responseMap;
        void getResponse( const Request& request );
        void postResponse( const Request& request );
        void deleteResponse( const Request& request );
        void invalidResponse( const Request& request );

        std::string prepPath(std::string requestedPath);

    public:
        Response( const Request& request, t_server serv, t_location location );
        const char *getResponse( void );
        int  getSize( void );

};


std::string timestamp(void);
# define FOF_PATH "./website/pages/404.html"
# define FOF(body, length)("HTTP/1.1 404 NotFound\n" timestamp() "Server: webserv\nContent-Length: " length "\nConnection: Closed\Content-Type: text/html\n\n" body)




#endif