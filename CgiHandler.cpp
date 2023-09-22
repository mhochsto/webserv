#include "CgiHandler.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"
#include "utils.hpp"

/*asuming all cgi files have an extension*/
std::string CgiHandler::getPathInfo(std::string path){
    
    path.erase(0, std::strlen(CGI_PATH) + 1);
    if (path.find_first_of('/') == std::string::npos) return "";
   path = path.substr(path.find_first_of('/'));
    return path;

}

/*
user authentication variables not supported and theirfor not set: AUTH_TYPE, REMOTE_USER, REMOTE_IDENT

REMOTE_HOST is set to REMOTE_ADDR if a server cannot retrieve REMOTE_HOST information.
Since the Subject forbids the use of the functions necceccery to retrieve these kind of informations,
we set REMOTE_HOST to REMOTE_ADDR by default.

Overview of available Variables:
https://www6.uniovi.es/~antonio/ncsa_httpd/cgi/env.html
*/
CgiHandler::CgiHandler(Response& response ,Request& request, t_server serv, std::string path, std::string rawUrlParameter): m_path(path){

    std::stringstream ssport;
    ssport << serv.port;

    m_env["SERVER_SOFTWARE"] = "webserv/1.0";
    m_env["SERVER_NAME"] = request.get("Host");
    m_env["GATEWAY_INTERFACE"] = "CGI/1.1";
    m_env["SERVER_PROTOCOL"] = request.getHttpVersion();
    m_env["SERVER_PORT"] = ssport.str();
    m_env["REQUEST_METHOD"] = request.getType();
    m_env["PATH_INFO"] = getPathInfo(path);
    m_env["PATH_TRANSLATED"] = serv.root + "/" + request.getPath();
    m_env["SCRIPT_NAME"] = path.substr(1, path.find(getPathInfo(path)) - 1);
    m_env["QUERY_STRING"] = rawUrlParameter;
    m_env["REMOTE_HOST"] = response.getClientAddr(); 
    m_env["REMOTE_ADDR"] = response.getClientAddr(); 
    m_env["CONTENT_TYPE"] = request.getBody();
	if (request.requestContains("Content-Length")){
    	m_env["CONTENT_LENGTH"] = request.get("Content-Length");
	}
	if (request.requestContains("Accept")){
    	m_env["HTTP_ACCEPT"] = request.get("Accept");
	}
	if (request.requestContains("User-Agent")){
    	m_env["HTTP_USER_AGENT"] = getFirstWord(request.get("User-Agent"));
	}
	(void)response;

}

void CgiHandler::execute(void){
	int fd[2];

	if (pipe(fd) == -1){
		throw std::runtime_error(SYS_ERROR("pipe"));
	 }

	char *argv[] = { (char *)m_path.c_str(), NULL};
	
	/* todo: create cgi env */
	extern char **environ;
	pid_t pid = fork();
	if (pid ==  -1) throw std::runtime_error(SYS_ERROR("fork"));
	if (pid == 0) {
		if (dup2(fd[1], STDOUT_FILENO) == -1) throw std::runtime_error(SYS_ERROR("dup2"));
		close (fd[0]);
		close (fd[1]);
		if (execve(m_path.c_str(), argv , environ) == -1)throw std::runtime_error(SYS_ERROR("execve"));
	} else {
		waitpid(pid, NULL, 0);
	}
	close(fd[1]);
	char buf[100];
	while(true){
		std::memset(buf, 0, 100);
		int rd = read(fd[0], buf, 100);
		if (rd == -1) throw std::runtime_error(SYS_ERROR("read"));
		if (rd == 0) break;
		m_output += buf;
	}
	close (fd[0]);
}

std::string CgiHandler::getOutput( void ) {return m_output;}
