#include "Server.hpp"
#include "Config.hpp"
#include "Response.hpp"
#include "Request.hpp"

int main(int argc, char **argv) {
    Config config("test_site.config");
    Server webserv(config.getServerConfig());
        webserv.run();
    try {
    }
    catch (std::exception &e){
        std::cerr << e.what() << std::endl;
    }
    return 0;

    (void)argc;
    (void)argv;
}