# include "CgiHandler.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "utils.hpp"

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
CgiHandler::CgiHandler( t_client& client ) : m_client(client) {
	client.lastAction = time(NULL);
	std::stringstream ssport;
	std::stringstream ssSize;
	ssport << client.request->getClient().config.port;
	ssSize << client.request->getBody().size();
	m_envStr.push_back("SERVER_SOFTWARE=webserv/1.0");
	m_envStr.push_back("SERVER_NAME=" + client.request->get("Host"));
	m_envStr.push_back("GATEWAY_INTERFACE=CGI/1.1");
	m_envStr.push_back("SERVER_PROTOCOL=HTTP/1.1");
	m_envStr.push_back("SERVER_PORT=" + ssport.str());
	m_envStr.push_back("REQUEST_METHOD=" + client.request->getType());
	m_envStr.push_back("PATH_INFO=" + client.request->getPathInfo());
	m_envStr.push_back("PATH_TRANSLATED=" + client.request->getPath().substr(1) + client.request->getPathInfo() + (client.request->getQueryString().empty() ? "" : "?" + client.request->getQueryString()));
	m_envStr.push_back("SCRIPT_NAME=" + client.request->getScriptName());
	m_envStr.push_back("QUERY_STRING=" + client.request->getQueryString());
	m_envStr.push_back("REMOTE_HOST=" + client.request->getClientIP());
	m_envStr.push_back("REMOTE_ADDR=" + client.request->getClientIP());
	m_envStr.push_back("CONTENT_TYPE=" + client.request->get("Content-Type"));
	m_envStr.push_back("CONTENT_LENGTH=" + ssSize.str());
	m_envStr.push_back("HTTP_ACCEPT=" + client.request->get("Accept"));
	m_envStr.push_back("HTTP_USER_AGENT=" + client.request->get("User-Agent"));
	for(std::vector<std::string>::iterator it = m_envStr.begin(); it != m_envStr.end(); ++it){
		m_envCharPtr.push_back((char *)it->c_str());
	}
	m_envCharPtr.push_back(NULL);
	execute();
}

std::string CgiHandler::findExecutablePath( std::string path ){

	m_extension = path.substr(1);
	m_extension.erase(0, path.find_last_of('.'));
	if (m_extension.find_first_of('/') != m_extension.find_last_of('/')){
		m_extension.erase(m_extension.find('/'));
	}
	if (m_extension == "js"){
		return "/usr/bin/node";
	}
	else if (m_extension == "py"){
		return "/usr/bin/python3";
	}
	return path;
}

void CgiHandler::prepSockets(void){
	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, m_in) == -1){
		throw std::runtime_error(SYS_ERROR("socketpair"));
	}
	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, m_out) == -1){
		close(m_in[0]);
		close(m_in[1]);
		throw std::runtime_error(SYS_ERROR("socketpair"));
	}
	m_client.socketVector->push_back((pollfd){m_in[WRITE], POLLIN, 0});
	m_client.socketVector->push_back((pollfd){m_out[READ], POLLOUT, 0});
}

void CgiHandler::execute( void ) {
	std::string executablePath = findExecutablePath(m_client.request->getPath());
	char *argv[] = {(char *)executablePath.c_str(), (char *)(m_client.request->getClient().location.cgiScript.empty() ? m_client.request->getPath().c_str() : NULL) , NULL};
	prepSockets();
	m_client.CgiPid = fork();
	if (m_client.CgiPid ==  -1) {
		throw std::runtime_error(SYS_ERROR("fork"));
	}
	if (m_client.CgiPid == 0) {
		signal(SIGINT, SIG_DFL);
		if (dup2(m_in[READ], STDIN_FILENO) == -1) throw std::runtime_error(SYS_ERROR("dup2"));
		if (dup2(m_out[WRITE], STDOUT_FILENO) == -1) throw std::runtime_error(SYS_ERROR("dup2"));
		close(m_in[READ]);
		close(m_out[WRITE]);
		closePollfds(m_client.request->getPollfds());
		if (execve(argv[0], argv , m_envCharPtr.data()) == -1){
			exit(1);
		}
	}
	close(m_in[READ]);
	close(m_out[WRITE]);

}

bool CgiHandler::isPipeFd(int fd){
	if (m_in[READ] == fd){
		return true;
	}
	if (m_in[WRITE] == fd){
		return true;
	}
	if (m_out[READ] == fd){
		return true;
	}
	if (m_out[WRITE] == fd){
		return true;
	}
	return false;
}


void CgiHandler::CgiSocketsToRemove( int *in, int *out ){
	*in = m_in[WRITE];
	*out = m_out[READ];
}

std::string& CgiHandler::getOutput( void ) {return m_output;}

const std::string& CgiHandler::getExtension( void ) { return m_extension;}


