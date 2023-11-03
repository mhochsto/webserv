

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <map>

#include <sys/stat.h> // stat()
#include <sys/types.h> // struct DIR; *dir()
#include <dirent.h> // struct dirent
# include <unistd.h> // access() && dup2


# include "DataStructs.hpp"
# include "utils.hpp"
# include "Error.hpp"

# define HTTP_HEADER_LIMIT 8192

class Request {
	public:
		Request(t_client& client, std::vector<pollfd>& pollfds);
		~Request();
	
		bool        contains( std::string str ) const;
		std::string get( std::string str ) const;
		const std::string& getType( void );
		const std::string& getPath( void );
		const std::string& getBody( void );
		const std::string& getInvalidRequest( void );
		const std::string& getClientIP( void );
		const std::string& getQueryString( void );
		const std::string& getPathInfo( void );
		const std::string& getScriptName( void );
		std::vector<pollfd>& getPollfds( void ); 
		t_client& 			getClient(void);
		bool				getIsCgi( void );
		bool				getShowDir( void );
		bool				getIsRedirect( void );
		void		setPath( std::string newPath );
		void		setInvalidRequest(std::string invalidRequest);
	private:
		t_client&					m_client;
		std::string	     			m_requestType;
		std::string	    	 		m_requestPath;
		std::string					m_invalidRequest;
		std::string		     		m_requestHttpVersion;
		std::string					m_requestBody;
		std::string					cgi_scriptName;
		std::string					cgi_queryString;
		std::string					cgi_pathInfo;
		std::vector<pollfd>&		cgi_pollfds;
		bool						cgi_isCgi;
		bool						m_showDir;
		bool						m_isRedirect;
		std::map<std::string, std::string> m_requestData;

		int 		validateAndSetRequestLine( const std::string& line );
		std::string getLocationName( void );
		int			parseHeader(void);
		void		setRedirects(void);
		void		setRoot( void );
		void		saveQueryString( void );
		void		setPathInfo(void);
		void		setIsCgi(void);
		void		checkFilePermissions(void);
		void		checkIfDirectoryShouldBeShown( void );
		void 		validateRequestType(const t_location& location);
		void		checkBodyLength(void);
		void		validateExtension(std::string& extension, t_location& location);
};

std::ostream    &operator<<(std::ostream &os, const Request &rhs);

#endif