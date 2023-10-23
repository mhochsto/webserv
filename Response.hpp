#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <algorithm>
# include <ctime>


#include <fcntl.h>

# include "Config.hpp"

class Request;

class Response {
    public:
        Response( t_client& client, Request& request );
        const char *returnResponse( void ) const;
        int  getSize( void ) const;
    private:
        t_client&   m_client;
        Request&    m_request;
        std::string m_response;
        int         m_responseSize;
        typedef void (Response::*funcPtr)(Request&);
        
        std::map<std::string, funcPtr> m_responseMap;
        void getResponse( Request& request );
        void postResponse( Request& request );
        void deleteResponse( Request& request );
        void putResponse( Request& request );

        std::string createStringFromFile(std::string fileName);
        std::string showDir(std::string path);
        void createResponse(std::string rspType, std::string file);
        void createErrorResponse( const std::string& errorCode );
        void executeCGI(void);

};


std::string timestamp(void);

# define FOF_PATH "./website/pages/404.html"

std::ostream    &operator<<(std::ostream &os, const Response &rhs);

#endif