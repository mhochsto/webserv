#ifndef ERROR_HPP
# define ERROR_HPP
#include <iostream>

# define WHITESPACE " \t\v\r"
# define SYS_ERROR(msg) ("Server::" msg " failed")
# define CONFIG_ERROR(msg)("Server::Config Error: " msg)

#endif