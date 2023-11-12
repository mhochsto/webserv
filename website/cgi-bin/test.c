#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv, char **env){
   // chdir("../");
    char buf[3000];
    getcwd(buf);

 //     printf("Content-type: text/html\r\n\r\n");
      printf("<!doctype html>");
     printf("<html>");
     printf("  <head>");
     printf("    <meta name ='viewport' content='width=device-width, initital-scale=1.0'>");
     printf("    <title>Thankyou Page</title>");
     printf("  </head>");
     printf("  <body>");
     printf("    <h1>%s</h1>", buf);
     printf("  </body>");
     printf("</html>");
}