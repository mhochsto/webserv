#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "Error.hpp"
# include "Config.hpp"

# include <unistd.h> // fork () && execve()
#include <sys/socket.h>
#include <sys/types.h>

# include <numeric>

class Response;
class Request;

#define READ 0
#define WRITE 1

class CgiHandler {
	public:
		CgiHandler(t_client& client);
		void execute( void );
		std::string& getOutput( void );
		t_client& getClient( void );
		bool isPipeFd(int fd);
		void CgiSocketsToRemove( int *in, int *out );
		const std::string& getExtension();

	private:
		t_client&							m_client;
		std::string							m_extension;
		std::string 						m_output;
		std::vector<std::string>			m_envStr;
		std::vector<char *>					m_envCharPtr;
		int 								m_in[2];
		int									m_out[2];								
		std::string getPathInfo(std::string path);
		std::string findExecutablePath( std::string path );
		void prepSockets(void);
};

#endif