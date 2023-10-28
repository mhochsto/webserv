#ifndef DEFAULTCONFIG_HPP
# define DEFAULTCONFIG_HPP

# define SERVER_LISTEN 8800
# define SERVER_LOCALHOST "localhost"
# define SERVER_CLIENT_MAX_BODY_SIZE 10000
# define SERVER_ROOT "/website/"
# define SERVER_INDEX "index.html"
# define SERVER_ALLOWED_METHODS "GET"
# define SERVER_ERROR_PAGE_400 "/pages/400.html"
# define SERVER_ERROR_PAGE_403 "/pages/403.html"
# define SERVER_ERROR_PAGE_404 "/pages/404.html"
# define SERVER_ERROR_PAGE_413 "/pages/413.html"
# define SERVER_ERROR_PAGE_405 "/pages/405.html"
# define SERVER_ERROR_PAGE_500 "/pages/500.html"
# define SERVER_ERROR_PAGE_502 "/pages/502.html"
# define SERVER_ERROR_PAGE_503 "/pages/503.html"
# define SERVER_ERROR_PAGE_505 "/pages/505.html"

# define ALLOWED_REQUESTS {"GET", "POST", "DELETE", "PUT", NULL}
# define CGI_PATH "./website/cgi-bin"

# define PORT_MIN 0
# define PORT_MAX 65535 // Port ranges: https://www.ibm.com/docs/en/ztpf/2020?topic=overview-port-numbers

# define CLIENT_BODY_SIZE_MIN 1
# define CLIENT_BODY_SIZE_Max 1000000 // = 1MB - https://docs.nginx.com/nginx-management-suite/acm/how-to/policies/request-body-size-limit/

#define UNSET -1

#define LAST_RESORT_404 "<!doctype html>\
<html>\
  <head>\
    <meta name ='viewport' content='width=device-width, initital-scale=1.0'>\
    <title>404 File not Found</title>\
  </head>\
  <body>\
    <h1><strong>404 File not Found</strong></h1>\
  </body>\
</html>"

#endif