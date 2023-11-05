
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

# include "DataStructs.hpp"
# include "utils.hpp"
# include "Error.hpp"

class configException : public std::exception
{	
   	public:
		configException(std::string errorMsg): msg(errorMsg){}
		~configException() throw () {} 
		const char* what() const throw() { return msg.c_str(); }
	private:
		std::string msg;
};

class Config{
	public:
		Config( std::string configFileName );
		~Config();
		std::vector<t_config>&		getServerConfig( void ) { return m_serverConfig;}

	private:
		typedef void (*fLocation)(std::string, t_location&);
		typedef void (*fConfig)(std::string, t_config&);
		
		std::vector<t_config>   			m_serverConfig;
		std::map < std::string, fLocation >	m_locationFunctions;
		std::map < std::string, fConfig > 	m_configFunctions;


		std::string					getBlock( std::string type, std::string& in );
		void						addLocation(std::string newLocation, t_config& serverConfig);
		void						addServerConfig(std::string& in);
		fLocation 					functionWrapper(fLocation ptr){ return ptr; }
		fConfig 					functionWrapper(fConfig ptr){ return ptr; }

	template <typename T>
	static void addIndex(std::string line, T& set){
		set.index = getFirstWord(line);
		if (!line.empty()){
			throw configException("invalid index in: [" + line + "]");
		}
	}

	template <typename T>
	static void addAllowedMethods(std::string line, T& set ){
		const char *allowedRequest[] = ALLOWED_REQUESTS;
		set.allowedMethods = convertStringtoVector(line, WHITESPACE);
		if (set.allowedMethods.size() == 0){
			throw configException("no methods listed: " + line);
		}
		for(std::vector<std::string>::iterator it = set.allowedMethods.begin(); it != set.allowedMethods.end(); ++it ) {
			bool valid = false;
			for (unsigned int i = 0; allowedRequest[i]; ++i){
				if (*it == allowedRequest[i]){
					valid = true;
					break ;
				}
			}
			if (valid == false){
				throw configException("invalid Method: [" + *it + "]");
			}
			valid = false;
		}
	}

	template <typename T>
	static void addProxyPass(std::string line, T& set ){
		set.proxyPass = line;
		removeFirstWord(line);
		if (!line.empty()){
			throw configException("invalid proxy_pass: [" + line + "]");
		}
		formatPath(set.proxyPass);
	}
	template <typename T>
	static void addAllowedCgiExtension(std::string line, T& set ){
		set.allowedCgiExtensions = convertStringtoVector(line, WHITESPACE);
		if (set.allowedCgiExtensions.size() == 0){
			throw configException("no extensions listed: " + line);
		}
		for (unsigned long i = 0; i < set.allowedCgiExtensions.size(); ++i){
			if (set.allowedCgiExtensions.at(i).at(0) != '.'){
				throw configException("invalid cgi extension (needs to start with '.'): " + set.allowedCgiExtensions.at(i));
			}
		}
	}

	template <typename T>
	static void addAutoIndex(std::string line, T& set ){
		if (line == "off") {
			set.autoIndex = false;
		}
		else if( line == "on") {
			set.autoIndex = true;
		}
		else {
			throw configException("invalid Autoindex: " + line);
		}
	}

	template <typename T>
	static void addClientMaxBodySize(std::string line, T& set ){
		char *endptr;
		double dValue = std::strtod(line.c_str(), &endptr);
		if (*endptr || dValue < 0 || dValue > std::numeric_limits<int>::max()){
			throw configException("invalid client_max_body_size in: [" + line + "]");
		}
		set.clientMaxBodySize = dValue;
	}

	template <typename T>
	static void 	addListen(std::string line, T& set ){
		std::string port = getFirstWord(line);
		if (!line.empty() || port.find(':') == std::string::npos){
			throw configException("invalid listen: " + port + line);
		}
		set.serverIP = port.substr(0, port.find(':'));
		port.erase(0, port.find(':') + 1);
		char *endptr;
		double dValue = std::strtod(port.c_str(), &endptr);
		if (*endptr || dValue < PORT_MIN || dValue > PORT_MAX ){
			throw configException("invalid port: [" + port + "]");
		}
		set.port = dValue;
	}

	template <typename T>
	static void 	addServerName(std::string line, T& set ){
		set.serverName.clear();
		std::stringstream sstream(line);
		std::string word;
		while ((std::getline(sstream, word, ' '))){
			set.serverName.push_back(getFirstWord(word));
			getFirstWord(line);
		}
		if (set.serverName.size() == 0){
			throw configException("no server name provided");
		}
	}

	template <typename T>
	static void 	addRoot(std::string line, T& set ){
		formatPath(line);
		set.root = line;
	}

	template <typename T>
	static void		addCgiName(std::string line, T& set){
		set.cgiScript = getFirstWord(line);
		formatPath(set.cgiScript);
		set.cgiScript.insert(0, ".");
		if (!line.empty()){
			throw configException("invalid CgiName: [" + line + "]");

		}
	}

	template <typename T>
	static void addRedirects(std::string line, T& set) {

		std::string key = getFirstWord(line);
		std::string value = getFirstWord(line);
		if (!line.empty() || key.empty() || value.empty()){
			throw configException("invalid errorPage: [" + line + "]");
		}

		if (set.redirects[key].size() != 0){
			set.redirects[key].clear();
		}
		formatPath(key);
		formatPath(value);
		set.redirects[key] = value;
	}

	template <typename T>
	static void addErrorPages(std::string line, T& set) {

		std::string key = getFirstWord(line);
		std::string value = getFirstWord(line);
		if (!line.empty() || key.empty() || value.empty()){
			throw configException("invalid errorPage: [" + line + "]");
		}

		if (set.errorPages[key].size() != 0){
			set.errorPages[key].clear();
		}
		formatPath(value);
		set.errorPages[key] = value;
	}

};
#endif