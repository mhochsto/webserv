#include "Config.hpp"
# include "Error.hpp"
# 
std::string Config::getBlock( std::string type, std::string& in ){
    if (in.find(type) == std::string::npos) throw std::runtime_error(CONFIG_ERROR("Server not defined"));
    
    std::stack<int> brackets;
    int i = in.find(type) + type.length();
    for (; i < in.length(); i++){
        if (in.at(i) == '{'){
            brackets.push(i);
        }
        else if (in.at(i) == '}'){
            brackets.pop();
            if (brackets.empty()){
                    break ;
            }
        }
    }
    if (i == in.length() && !brackets.empty()) throw std::runtime_error(CONFIG_ERROR("Open brackets in file"));
    
    std::string currentScope = in.substr(in.find(type) + type.length() + 1, i); 
    in.erase(in.find(type), i);
    return (currentScope);
}

Config::Config(std::string configFileName ){
    std::fstream infile(configFileContent.c_str());
    if (!infile) throw std::runtime_error(SYS_ERROR("open"));
    
    std::stringstream sstream;
    sstream << infile.rdbuf();
    std::string input = sstream.str();
    infile.close();
    getBlock("Server", input);

}