
#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <string>
#include <map>
# include <fstream>
# include <sstream>
# include <stack>

class Config{
    private:
        int m_port;
        std::string m_serverName;
        std::string m_hostname;
        long int m_clientMaxBodySize;
        std::string m_root;
        std::string m_index;
        std::map<std::string, std::string> m_errorPages;
        std::string configFileContent;
    public:
        Config( std::string configFileName );
std::string getBlock( std::string type, std::string& in );
};

#endif