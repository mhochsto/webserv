#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "Error.hpp"
# include "Config.hpp"

# include <unistd.h> // fork () && execve()
# include <sys/wait.h> // waitpid()
# include <numeric>
class Response;
class Request;

class CgiHandler {
    private:
        std::string m_path;
        std::string m_output;
        std::map<std::string, std::string> m_env;
        char        **m_args;
        char **convertEnv( void );
        char **convertArgs( void );
        std::string getPathInfo(std::string path);

    public:
        CgiHandler(Response& response ,Request& request, t_server serv, std::string path, std::string rawUrlParameter);
        void execute( void );
        std::string getOutput( void );
};

#endif