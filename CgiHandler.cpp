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

void CgiHandler::execute( void ) {
	char *argv[] = { (char *)(m_request.getClient().location.cgiScript.empty() ? m_request.getPath().c_str() : m_request.getClient().location.cgiScript.c_str()) , NULL};
	int inFile = open(".tempfile0", O_CREAT | O_RDWR | O_CLOEXEC, 0664);
	int outFile = open(".tempfile1", O_CREAT | O_RDWR | O_CLOEXEC, 0664);
	std::cout << "executing: " << argv[0] << std::endl;
	if (m_request.getType() == "POST") {
		write(inFile, m_request.getBody().c_str(), m_request.getBody().size());
	}
	pid_t pid = fork();
	if (pid ==  -1)																throw std::runtime_error(SYS_ERROR("fork"));
	if (pid == 0) {
		if (dup2(outFile, STDOUT_FILENO) == -1)								throw std::runtime_error(SYS_ERROR("dup2"));
		if (dup2(inFile, STDIN_FILENO) == -1)									throw std::runtime_error(SYS_ERROR("dup2"));
		close(inFile);
		close(outFile);
		if (execve(argv[0], argv , m_envCharPtr.data()) == -1){
			perror("execve");
		}
	}
	waitpid(pid, NULL, 0);
	close(outFile);
	close(inFile);
	
	char buf[100];
	outFile = open(".tempfile1", O_RDWR, 0664);
	while(true){
		std::memset(buf, 0, 100);
		int rd = read(outFile, buf, 100);
		if (rd == -1)															throw std::runtime_error(SYS_ERROR("read"));
		if (rd == 0) break;
		m_output += buf;
	}
	close(outFile);
	remove(".tempfile0");
	remove(".tempfile1");
}

std::string CgiHandler::getOutput( void ) {return m_output;}



