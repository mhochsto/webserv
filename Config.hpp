
#ifndef CONFIG_HPP
# define CONFIG_HPP

# include <map>
# include <fstream>
# include <sstream>
# include <stack>
# include <vector>
# include <cstring>
# include <cstdlib>
# include <set>
# include <limits>

# include <unistd.h> // access() && dup2

# include "DataStructs.hpp"
# include "utils.hpp"
# include "Error.hpp"

class configException : public std::exception
{
	private:
		std::string msg;
   	public:
		configException(std::string errorMsg): msg(errorMsg){}
		~configException() throw () {} 
		const char* what() const throw() { return msg.c_str(); }
};

class Config{
	typedef void (Config::*funcPtr)(std::string, std::vector<std::string>&);

	private:
		std::set<std::string> 			allowedIdentifiersLocation;
		std::set<std::string> 			allowedIdentifiersServer;
		std::vector<t_config>   		m_serverConfig;
		std::map<std::string, funcPtr>	m_functionMap;

		std::string					getBlock( std::string type, std::string& in );
		void						addIndex(std::string line, std::vector<std::string>& set);
		void						addAllowedMethods(std::string line, std::vector<std::string>& set );
		void						addAllowedCgiExtension(std::string line, std::vector<std::string>& set );
		void						addClientMaxBodySize(std::string line, std::vector<std::string>& set );
		void 						addAutoIndex(std::string line, std::vector<std::string>& set );
		void 						addProxyPass(std::string line, std::vector<std::string>& set );
		void 						addListen(std::string line, std::vector<std::string>& set );
		void 						addServerName(std::string line, std::vector<std::string>& set );
		void 						addRoot(std::string line, std::vector<std::string>& set );
		void						addEntry(std::string line, vectorMap& errorPages, bool urlFormat);
		void						addLocation(std::string newLocation, t_config& serverConfig);
		void						addServerConfig(std::string& in);
	public:
		Config( std::string configFileName );
		~Config();

		std::vector<t_config>		getServerConfig( void );
};
#endif