#include "CgiHandler.hpp"
#include "Request.hpp"
#include "Response.hpp"
# include "Config.hpp"

/*asuming all cgi files have an extension*/
std::string CgiHandler::getPathInfo(std::string path){
    
    path.erase(0, std::strlen(CGI_PATH) + 1);
    if (path.find_first_of('/') == std::string::npos) return "";
   path = path.substr(path.find_first_of('/'));
    return path;

}

CgiHandler::CgiHandler(Response& response ,Request& request, t_server serv, std::string path): m_path(path){

    std::stringstream ssport;
    ssport << serv.port;
    getPathInfo(path);
/*
    m_env["SERVER_SOFTWARE"] = "webserv/1.0";
    m_env["SERVER_NAME"] = requestData["Host"];
    m_env["GATEWAY_INTERFACE"] = "CGI/1.1";
    m_env["SERVER_PROTOCOL"] = request.getHttpVersion();
    m_env["SERVER_PORT"] = ssport.str();
    m_env["REQUEST_METHOD"] = request.getType();
    m_env["PATH_INFO"] = getPathInfo(path);
    m_env["PATH_TRANSLATED"] = serv.root + "/" + request.getPath();
    m_env["SCRIPT_NAME"] = 
    m_env["QUERY_STRING"] = 
    m_env["REMOTE_HOST"] = 
    m_env["REMOTE_ADDR"] = 
    m_env["AUTH_TYPE"] = 
    m_env["REMOTE_USER"] = 
    m_env["REMOTE_IDENT"] = 
    m_env["CONTENT_TYPE"] = 
    m_env["CONTENT_LENGTH"] = 
    m_env["HTTP_ACCEPT"] = 
    m_env["HTTP_USER_AGENT"] = 

*/
(void)response;
(void)serv;
(void)request;
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
	std::string content;
	while(true){
		std::memset(buf, 0, 100);
		int rd = read(fd[0], buf, 100);
		if (rd == -1) throw std::runtime_error(SYS_ERROR("read"));
		if (rd == 0) break;
		content += buf;
	}
	close (fd[0]);
}

std::string CgiHandler::getOutput( void ) {return m_output;}
