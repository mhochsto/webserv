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
		Request&							m_request;
		std::string 						m_output;
		std::vector<std::string>			m_envStr;
		std::vector<char *>			m_envCharPtr;

		/*private Memberfuntion*/
		std::string getPathInfo(std::string path);

	public:
		/*Con- and Destructors*/
		CgiHandler(Request& request);

		/*Memberfunctions*/
		void execute( void );
		std::string getOutput( void );
};


#endif