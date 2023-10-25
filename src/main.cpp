#include "Server.hpp"
#include "Config.hpp"
#include "Response.hpp"
#include "Request.hpp"

int main(int argc, char **argv) {
    Config config("webserv.config");
    Server webserv(config.getServerConfig());
    try {
        webserv.run();
    }
    catch (std::exception &e){
        std::cerr << e.what() << std::endl;
    }
    return 0;

    (void)argc;
    (void)argv;
}