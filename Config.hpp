
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
# include <unistd.h> // access() && dup2

# define ALLOWED_REQUESTS {"GET", "POST", "DELETE"}
# define ALLOWED_REQUESTS_COUNT 3

# define ALLOWED_DOMAIN_EXTENSIONS {".com", ".org", ".net", ".at", ".de"}
# define ALLOWED_DOMAIN_EXTENSIONS_COUNT 5

# define PORT_MIN 0
# define PORT_MAX 65535 // Port ranges: https://www.ibm.com/docs/en/ztpf/2020?topic=overview-port-numbers

#define CLIENT_BODY_SIZE_MIN 1
#define CLIENT_BODY_SIZE_Max 1000000 // = 1MB - https://docs.nginx.com/nginx-management-suite/acm/how-to/policies/request-body-size-limit/

#define CGI_PATH "./website/cgi-bin"


typedef struct s_location {
    std::vector<std::string>    allowed_methods;
    bool                        autoIndex;
    std::string                 index;
    std::string                 path;
} t_location;

typedef struct s_server {
    int                                 port;
    std::set<std::string>               serverName;
    std::string                         hostname;
    int                                 clientMaxBodySize;
    std::string                         root;
    std::string                         index;
    std::map<std::string, std::string>  errorPages;
    std::map<std::string, t_location>   locations;
} t_server;

class Config{
    private:
        std::string             m_configFileContent;
        std::vector<t_server>   m_servers;


        typedef void (Config::*funcPtr)(std::string, t_server&);
    public:
        Config( std::string configFileName );
        Config( const Config &cpy);
        ~Config();

        void        addServer(std::string& in);
        void        addLocation(std::string newLocation, t_server& serv);
        std::string getBlock( std::string type, std::string& in );
        void        fillServerStruct(std::string newServer, t_server& serv);

        std::string getFirstWord(std::string str);
        std::string validateValueFormat(std::string str);
        void        validateServerName(std::string domain);

        void        addMethodsLocation(std::string line, t_location& location);
        void        addAutoindexLocation(std::string line, t_location& location);
        void        addIndexLocation(std::string line, t_location& location);

        void	    addServerNameServer(std::string line, t_server& serv);
        void	    addHostServer(std::string line, t_server& serv);
        void	    addClientMaxBodySize(std::string line, t_server& serv);
        void	    addErrorPageServer(std::string line, t_server& serv);
        void	    addListenServer(std::string line, t_server& serv);
        
        /* didn't use absolut path since getcwd is forbidden */
        void	    addRootServer(std::string line, t_server& serv);
        void	    addIndexServer(std::string line, t_server& serv);

        std::vector<t_server>   getServerConfig( void );



};


#endif