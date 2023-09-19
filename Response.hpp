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
#include <sys/types.h> // struct DIR; *dir()
#include <dirent.h> // struct dirent

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
        
        void badRequestResponse( const Request& request );
        void methodNotAllowedResponse( const Request& request );
        
        std::string showDir(std::string path);
        void createResponse(std::string rspType, std::string file);

    public:
        Response( const Request& request, t_server serv, t_location location );
        const char *returnResponse( void );
        int  getSize( void );

};


std::string timestamp(void);
# define FOF_PATH "./website/pages/404.html"




#endif