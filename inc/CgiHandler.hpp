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
	public:
		CgiHandler(Request& request);
		void execute( void );
		std::string getOutput( void );
	private:
		Request&							m_request;
		std::string 						m_output;
		std::vector<std::string>			m_envStr;
		std::vector<char *>					m_envCharPtr;

		std::string getPathInfo(std::string path);
		std::string findExecutablePath( std::string path );
};

#endif