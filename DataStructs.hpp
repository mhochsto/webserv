#ifndef DATASTRUCTS_HPP
# define DATASTRUCTS_HPP

typedef struct s_location {
    bool                        autoIndex;
    ssize_t                     clientMaxBodySize;
    std::string                 index;
    std::string                 path;
    std::string                 proxyPass;
    std::vector<std::string>    allowed_methods;
    std::vector<std::string>    allowed_cgi_extension;

} t_location;

typedef struct s_server {
    int                                 port;
    std::set<std::string>               serverName;
    std::string                         hostname;
    ssize_t                             clientMaxBodySize;
    std::string                         root;
    std::string                         index;
    std::map<std::string, std::string>  errorPages;
    std::map<std::string, t_location>   locations;
    int                                 fd;
} t_server;


#endif