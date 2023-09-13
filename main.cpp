#include "Server.hpp"
#include "Config.hpp"

int main() {

    Config("webserv.config");
    
    return 1;

    /* set up server with values from config*/
    Server webserv;
    webserv.run();
    return 0;
}