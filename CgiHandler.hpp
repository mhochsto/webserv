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
		/*private Variables*/
		std::string							m_path;
		std::string							m_type;
		std::string							m_requestBody;
		std::string 						m_output;
		std::map<std::string, std::string>	m_env;

		/*private Memberfuntion*/
		std::string getPathInfo(std::string path);

	public:
		/*Con- and Destructors*/
		CgiHandler(Response& response ,Request& request, t_config serv, std::string path, std::string rawUrlParameter);

		/*Memberfunctions*/
		void execute( void );
		std::string getOutput( void );
};

#endif