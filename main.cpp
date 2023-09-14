#include "Server.hpp"
#include "Config.hpp"

int main(int argc, char **argv) {

    
    
    Config config("webserv.config");
    Server webserv(config);
    webserv.run();
    return 0;

    (void)argc;
    (void)argv;
}