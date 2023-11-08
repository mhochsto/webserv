#include "Server.hpp"
#include "Config.hpp"
#include "Response.hpp"
#include "Request.hpp"

int main(int argc, char **argv) {


    if (argc > 2){
        std::cerr << "Please provide only one Input file\n";
        return 0;
    }
    std::string file;
    if (argc == 2){
        file = argv[1];
    }
    else {
        file = DEFAULT_CONFIG_FILE;
    }

    if (access(file.c_str(), R_OK) != 0){
        std::cerr << "Cannot access file\n";
        return 2;
    }
    Config config(file);

    Server webserv(config.getServerConfig());
    try {
        webserv.run();
    }
    catch (std::exception &e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}