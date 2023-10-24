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
CgiHandler::CgiHandler( Request& request ) : m_request(request) {

	std::stringstream ssport;
	std::stringstream ssSize;
	ssport << request.getClient().config.port;
	ssSize << request.getBody().size();
	m_envStr.push_back("SERVER_SOFTWARE=webserv/1.0");
	m_envStr.push_back("SERVER_NAME=" + request.get("Host"));
	m_envStr.push_back("GATEWAY_INTERFACE=CGI/1.1");
	m_envStr.push_back("SERVER_PROTOCOL=HTTP/1.1");
	m_envStr.push_back("SERVER_PORT=" + ssport.str());
	m_envStr.push_back("REQUEST_METHOD=" + request.getType());
	m_envStr.push_back("PATH_INFO=" + request.getPathInfo());
	m_envStr.push_back("PATH_TRANSLATED=" + request.getClient().config.root + "/" + request.getPath());
	m_envStr.push_back("SCRIPT_NAME=" + request.getScriptName());
	m_envStr.push_back("QUERY_STRING=" + request.getQueryString());
	m_envStr.push_back("REMOTE_HOST=" + request.getClientIP());
	m_envStr.push_back("REMOTE_ADDR=" + request.getClientIP());
	m_envStr.push_back("CONTENT_TYPE=html");
	m_envStr.push_back("CONTENT_LENGTH=" + ssSize.str());
	m_envStr.push_back("HTTP_ACCEPT=" + request.get("Accept"));
	m_envStr.push_back("HTTP_USER_AGENT=" + request.get("User-Agent"));
	for(std::vector<std::string>::iterator it = m_envStr.begin(); it != m_envStr.end(); ++it){
		m_envCharPtr.push_back((char *)it->c_str());
	}
	m_envCharPtr.push_back(NULL);
	execute();
}

std::string CgiHandler::prepFilePath( std::string path ){
	std::string extension = path.substr(path.find_first_of('.'));
	if (extension.find_first_of('/') != extension.find_last_of('/')){
		extension.erase(extension.find('/'));
	}
	if (extension == ".js"){
		path.insert(0, "node ");
	}
	else if (extension == ".py"){
		path.insert(0, "python3 ");
	}
	return path;
}

void CgiHandler::execute( void ) {
	std::string filePath = prepFilePath(m_request.getPath());

	char *argv[] = { (char *)(m_request.getClient().location.cgiScript.empty() ? filePath.c_str() : m_request.getClient().location.cgiScript.c_str()) , NULL};
	int fd[2];
	std::cout << "reached  cgi part\n";
	if (m_request.getType() == "POST") {
		write(fd[0], m_request.getBody().c_str(), m_request.getBody().size());
	}
	if (pipe(fd)){
		throw std::runtime_error(SYS_ERROR("pipe"));
	}
	pid_t pid = fork();
	if (pid ==  -1) {
		throw std::runtime_error(SYS_ERROR("fork"));
	}
	if (pid == 0) {

		dup2(fd[0], STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		closePollfds(m_request.getPollfds());
		if (execve(argv[0], argv , m_envCharPtr.data()) == -1){
			perror("execve");
		}
	}
	close(fd[1]);

	char buf[1024];
	int rd = 1;
	while (rd) {
		std::memset(buf, 0, sizeof(buf));
		rd = read(fd[0], buf, sizeof(buf));
		if (rd <= 0){
			break ;
		}
		m_output += buf;
	}
	close(fd[0]);
	waitpid(pid, NULL, 0);

	remove(".tempfile0");
	remove(".tempfile1");
}

std::string CgiHandler::getOutput( void ) {return m_output;}



