
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


# define ALLOWED_REQUESTS {"GET", "POST", "DELETE", "PUT", NULL}
# define ALLOWED_REQUESTS_COUNT 4

# define ALLOWED_DOMAIN_EXTENSIONS {".com", ".org", ".net", ".at", ".de"}
# define ALLOWED_DOMAIN_EXTENSIONS_COUNT 5

# define PORT_MIN 0
# define PORT_MAX 65535 // Port ranges: https://www.ibm.com/docs/en/ztpf/2020?topic=overview-port-numbers

#define CLIENT_BODY_SIZE_MIN 1
#define CLIENT_BODY_SIZE_Max 1000000 // = 1MB - https://docs.nginx.com/nginx-management-suite/acm/how-to/policies/request-body-size-limit/

#define CGI_PATH "./website/cgi-bin"

# include "DataStructs.hpp"
# include "DefaultConfig.hpp"
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
	private:
		typedef void (Config::*serverFuncPtr)(std::string, t_server&);
		typedef void (Config::*locationFuncPtr)(std::string, void *);

		std::vector<t_server>   m_servers;
		std::map< std::string, locationFuncPtr > m_locationFunctions;
		std::map< std::string, serverFuncPtr> m_serverFunctions; 

	public:
		Config( std::string configFileName );
		~Config();

		void                        addLocation(std::string newLocation, t_server& serv);
		void                        addServer(std::string& in);
		void                        defaultConfigServer(t_server &serv);
		void						addtoLocationIfAllowed(std::string& line, std::map< std::string, std::pair<locationFuncPtr, void *> >& locationFunctions);


		/* helpers */
		std::vector<std::string>    convertStringtoVector(std::string str, std::string delimiter);
		void                        removeFirstWord(std::string& str);
		std::string                 getBlock( std::string type, std::string& in );
		void                        fillServerStruct(std::string newServer, t_server& serv);
		void                        formatPath(std::string& str);

		std::vector<t_server>       getServerConfig( void );


		/****************************************************************************/
		/*	Since alot of keywords can be present in both, server and location,		*/
		/*  we use Templates to reuse the functions.	 							*/
		/*	The FunctionWrapper is necessary to save all functions in a Map.		*/
		/*	Function templates need T, but since we cannot use T as Map Value we 	*/
		/*  use a void * to pass the pointer to the value we want to set 			*/
		/****************************************************************************/
		template <typename T>
		class FunctionWrapper {
			typedef void (Config::*FuncPtr)(std::string, T *);
			typedef void (Config::*retPtr)(std::string, void *);
			private:
				FuncPtr m_ptr;
			public:
				FunctionWrapper(FuncPtr ptr): m_ptr(ptr) {}
				retPtr getPtr(void) {
					return (retPtr)m_ptr;
				}
		};

		template< typename T>
		void addIndexLocation(std::string line, T *to_set){
			*to_set = line;
			removeFirstWord(line);
			if (!line.empty()){
				throw configException("invalid index in location: [" + line + "]");
			}
		}

		template< typename T>
		void addAllowedMethodsLocation(std::string line, T *to_set ){
			const char *allowedRequest[] = ALLOWED_REQUESTS;

			*to_set = convertStringtoVector(line, WHITESPACE);
			if ((*to_set).size() == 0){
				throw configException("no methods listed: " + line);
			}
			for(std::vector<std::string>::iterator it = (*to_set).begin(); it != (*to_set).end(); ++it ) {
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

		template< typename T>
		void addProxyPassLocation(std::string line, T *to_set ){
			*to_set = line.substr(0, line.find_first_of(WHITESPACE));
			line.erase(0, (*to_set).length() + 1);
			if (!line.empty()){
				throw configException("invalid proxy_pass: [" + line + "]");
			}
			formatPath(*to_set);
		}

		template< typename T>
		void addAllowedCgiExtensionLocation(std::string line, T *to_set ){
			*to_set = convertStringtoVector(line, WHITESPACE);
			if ((*to_set).size() == 0){
				throw configException("no extensions listed: " + line);
			}

			for (unsigned long i = 0; i < (*to_set).size(); ++i){
				if ((*to_set).at(i).at(0) != '.'){
					throw configException("invalid cgi extension (needs to start with '.'): " + (*to_set).at(i));
				}
			}
		}

		template< typename T>
		void addAutoIndexLocation(std::string line, T *to_set ){
			if (line == "off") {
				*to_set = false;
			}
			else if (line == "on") {
				*to_set = true;
			}
			else {
				throw configException("invalid auto_index_option: " + line);
			}
		}

		template< typename T>
		void addClientMaxBodySizeLocation(std::string line, T *to_set ){
			char *endptr;
			double dValue = std::strtod(line.c_str(), &endptr);
			if (*endptr || dValue < 0 || dValue > std::numeric_limits<int>::max()){
				throw configException("invalid client_max_body_size in location: [" + line + "]");
			}
			*to_set = dValue;
		}

};


#endif