#include "CgiHandler.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"
#include "utils.hpp"

/*asuming all cgi files have an extension*/
/*this should cut everything after the scriptname but nothing in the query part!
	but why isn't it like it just cuts the PATH too the script not the script 
	and the QUERY Part isn't cuted at all*/
std::string CgiHandler::getPathInfo(std::string path) {
	path.erase(0, std::strlen(CGI_PATH) + 1);
	if (path.find_first_of('/') == std::string::npos) return ("");
	path = path.substr(path.find_first_of('/'));
	return (path);
}

/*user authentication variables not supported and theirfor not set: AUTH_TYPE, REMOTE_USER, REMOTE_IDENT

REMOTE_HOST is set to REMOTE_ADDR if a server cannot retrieve REMOTE_HOST information.
Since the Subject forbids the use of the functions necceccery to retrieve these kind of informations,
we set REMOTE_HOST to REMOTE_ADDR by default.

Overview of available Variables:
https://www6.uniovi.es/~antonio/ncsa_httpd/cgi/env.html */
CgiHandler::CgiHandler( Response& response, Request& request, t_config serv,
						std::string path, std::string rawUrlParameter ) : m_path(path) {

	/*filling the Map for the env*/
	std::stringstream ssport;
	ssport << serv.port;
	m_env["SERVER_SOFTWARE"] = "webserv/1.0";
	m_env["SERVER_NAME"] = request.get("Host");
	m_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	m_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	m_env["SERVER_PORT"] = ssport.str();
	m_env["REQUEST_METHOD"] = request.getType();
	m_env["PATH_INFO"] = getPathInfo(path);
	m_env["PATH_TRANSLATED"] = serv.root + "/" + request.getPath();
	m_env["SCRIPT_NAME"] = path.substr(1, path.find(getPathInfo(path)) - 1);
	m_env["QUERY_STRING"] = rawUrlParameter;
	m_env["REMOTE_HOST"] = request.getClientIP();
	m_env["REMOTE_ADDR"] = request.getClientIP();
	m_env["CONTENT_TYPE"] = request.getBody();
	if (request.contains("Content-Length")) {
		m_env["CONTENT_LENGTH"] = "22";//request.get("Content-Length");
	}
	if (request.contains("Accept")){
		m_env["HTTP_ACCEPT"] = request.get("Accept");
	}
	if (request.contains("User-Agent")) {
		std::string userAgent = request.get("User-Agent");
		getFirstWord(userAgent);
		m_env["HTTP_USER_AGENT"] = userAgent;
	}

	/*other needed Information*/
	m_type = request.getType();
	m_requestBody = request.getBody();

	/*just some tests*/
	std::cout << std::endl << std::endl << "________________________________________________________________________" << std::endl;
	std::cout << "Everything I have inside the CGI_function!" << std::endl;
	std::cout << "rawUrlParameter:" << rawUrlParameter << std::endl;
	std::cout << "PATH_INFO: " << m_env["PATH_INFO"] << std::endl;
	std::cout << "Everything I get in the CGI function!" << std::endl;
	std::cout << response << std::endl;
	std::cout << request << std::endl;
	std::cout << serv;
	std::cout << "________________________________________________________________________" << std::endl << std::endl;
	/*delete everything above later*/
	execute();
}

void CgiHandler::execute( void ) {

	/*preping pipes for giving input to and output from the CGI-Script*/
	int in_fd[2];
	int out_fd[2];
	if (pipe(in_fd) == -1 || pipe(out_fd) == -1)								throw std::runtime_error(SYS_ERROR("pipe"));

	/*preping the path"Vector" to execute*/
	char *argv[] = { (char *)m_path.c_str(), NULL};

	/*preping the environment*/
	char	*env[this->m_env.size() + 1];
	std::vector<std::string> tmp;
	for (stringMap::iterator it = m_env.begin(); it != m_env.end(); it++) {
		tmp.push_back(it->first + "=" + it->second);
	}
	unsigned long i = 0;
	for (; i < m_env.size(); i++) {
		env[i] = (char *)tmp[i].c_str();
		// std::cout << env[i] << std::endl;
	}
	env [i] = NULL;

	/*preping input for POST Request`s*/
	if (m_type == "POST") {
		write(in_fd[1], m_requestBody.c_str(), m_requestBody.size());
	}
	if(close (in_fd[1]) == -1)													throw std::runtime_error(SYS_ERROR("close"));

	/*starting the execution*/
	pid_t pid = fork();
	if (pid ==  -1)																throw std::runtime_error(SYS_ERROR("fork"));
	if (pid == 0) {
		if (dup2(out_fd[1], STDOUT_FILENO) == -1)								throw std::runtime_error(SYS_ERROR("dup2"));
		if (dup2(in_fd[0], STDIN_FILENO) == -1)									throw std::runtime_error(SYS_ERROR("dup2"));
		if (close (out_fd[0]) == -1 || close (out_fd[1]) == -1)					throw std::runtime_error(SYS_ERROR("close"));
		if (close (in_fd[0]) == -1)												throw std::runtime_error(SYS_ERROR("close"));
		if (execve(argv[0], argv , env) == -1)									throw std::runtime_error(SYS_ERROR("execve"));//delete later //but why???
	} else {
		waitpid(pid, NULL, 0);
	}
	if (close(out_fd[1]) == -1 || close(in_fd[0]) == -1)						throw std::runtime_error(SYS_ERROR("close"));
	char buf[100];
	while(true){
		std::memset(buf, 0, 100);
		int rd = read(out_fd[0], buf, 100);
		if (rd == -1)															throw std::runtime_error(SYS_ERROR("read"));
		if (rd == 0) break;
		m_output += buf;
	}
	if (close (out_fd[0])) 														throw std::runtime_error(SYS_ERROR("close"));
}

std::string CgiHandler::getOutput( void ) {return m_output;}
