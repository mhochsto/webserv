#include "Server.hpp"
#include "Config.hpp"

int main() {

    Config("webserv.config");
    

    return 1;


    Server webserv;
    webserv.run();
    return 0;
}