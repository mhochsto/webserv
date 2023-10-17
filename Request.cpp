#include "Request.hpp"

/* function call order is important here. */
/**/
Request::Request(t_client& client): m_client(client), cgi_isCgi(false), m_showDir(false) {

	if (parseHeader()){
		return ;
	}
	setRedirects();
	setRoot();
	setPathInfo();
	setIsCgi();
	checkFilePermissions();
	if (!m_invalidRequest.empty()){
		return ;
	}
	checkIfDirectoryShouldBeShown();
}

Request::~Request(){}

void	Request::checkIfDirectoryShouldBeShown( void ) {
	struct stat statbuf;
	std::memset(&statbuf, 0 , sizeof(struct stat));
	if (stat(m_requestPath.c_str(), &statbuf) == -1){
		throw std::runtime_error(SYS_ERROR("stat"));
	}
	if (S_ISDIR(statbuf.st_mode)){
		if (m_client.location.autoIndex){
			m_showDir = true;
			return ;
		}
		else if (m_requestPath == ("." + m_client.config.root + "/") && !m_client.config.index.empty()){
			m_requestPath.append(m_client.config.index);
		}
		else if (!m_client.location.index.empty()){
			m_requestPath.append(m_client.location.index);

		}
		else {
			m_invalidRequest = "403 Forbidden\n";
			return ;
		}
	}
}

void Request::checkFilePermissions(void){
	if (cgi_isCgi){
		if (!m_client.location.cgiScript.empty()){
			m_requestPath = m_requestPath.at(0) != '.' ? m_client.location.cgiScript : "." + m_client.location.cgiScript;
		}
		if (access(m_requestPath.c_str(), X_OK) == -1) {
			m_invalidRequest = "403 Forbidden\n";
		}
	}
	else if (access(m_requestPath.c_str(), F_OK) == -1){
		m_invalidRequest = "404 NotFound\n";
	}
	else if (access(m_requestPath.c_str(), R_OK) == -1){
		m_invalidRequest = "403 Forbidden\n";
	}
}

void Request::setIsCgi(void){
	if ( !std::strncmp(m_client.location.path.c_str(), "/*", 2) || !std::strncmp(CGI_PATH, m_requestPath.c_str(), std::strlen(CGI_PATH))){
		cgi_isCgi = true;
		if (m_invalidRequest == "405 Method Not Allowed\n") {
			m_invalidRequest.clear();
			validateRequestType(m_client.config.locations[closestMatchingLocation(m_client.config.locations , m_requestPath.substr(1))]);
		}
	}
}

void Request::setPathInfo(void){
	std::string pathInfo = m_requestPath.substr(1);
	if (pathInfo.find('.') == std::string::npos) {
		return ;
	}
	pathInfo.erase(0, pathInfo.find('.') + 1);
	if (pathInfo.find('/') != std::string::npos) {
		cgi_pathInfo = pathInfo.substr(pathInfo.find('/'));
		m_requestPath.resize(m_requestPath.find(cgi_pathInfo));
	}
}

void		Request::saveQueryString( void ) {
	if (m_requestPath.find('?') == std::string::npos){
		return ;
	}
	cgi_queryString = m_requestPath.substr(m_requestPath.find('?') + 1);
	m_requestPath.resize(m_requestPath.find('?'));
}


void Request::setRoot( void ) {
	if (m_client.location.root.empty()){
		m_requestPath.insert(0, "." + m_client.config.root);
	}
	else {
		m_requestPath.erase(0, m_client.location.path.length());
		m_requestPath.insert(0, "." + m_client.location.root + (m_requestPath.empty() ? "/" : ""));
	}
}

/* changes requestPath to the redirection Path; 
we re-set the location, in case of a different location block for the "new" url */
void Request::setRedirects( void ){
	if (m_client.config.redirects.find(m_requestPath) != m_client.config.redirects.end()){
		m_requestPath = m_client.config.redirects[m_requestPath];
	}
	m_client.location = m_client.config.locations[getLocationName()];
}

void Request::validateRequestType(const t_location& location){
	std::vector <std::string> allowedMethods = location.allowedMethods.size() == 0 ? m_client.config.locations["/"].allowedMethods : location.allowedMethods;
	for (unsigned long i = 0; i < allowedMethods.size(); i++) {
		if (m_requestType == location.allowedMethods.at(i)){
			return ;
		}
	}
	m_invalidRequest = "405 Method Not Allowed\n";
}

int Request::validateAndSetRequestLine( const std::string& line ) {
	std::stringstream sstream(line);
	std::vector<std::string> vec;
	std::string word;
	while (std::getline(sstream, word, ' ')){
		vec.push_back(word);
		if (vec.size() == 4) break;
	}
	if (vec.size() != 3) {
		m_invalidRequest = "400 Bad Request\n";
		return 1;
	}
	m_requestType = vec.at(0);
	m_requestPath = vec.at(1);
	m_requestHttpVersion = vec.at(2);
	if ( !std::strcmp(m_requestHttpVersion.c_str(), "HTTP/1.1\r\n")){
		m_invalidRequest = "405 HTTP Version Not Supported\n";
		return 1;
	}
	saveQueryString();
	m_client.location = m_client.config.locations[getLocationName()];
	validateRequestType(m_client.location);
	return 0;
}

int Request::parseHeader(void) {
	std::string buffer = m_client.header;
	std::stringstream sstreamBuffer(buffer);
	std::string line;
	std::getline(sstreamBuffer, line);
	if (validateAndSetRequestLine(line)){
		return 1;
	}
	buffer.erase(0, line.length() + 1);
	while (std::getline(sstreamBuffer, line)){
		if (line.find_first_of(':') != std::string::npos){
			std::string value = line.substr(line.find_first_of(':') + 1);
			m_requestData[line.substr(0, line.find_first_of(':'))] = value.erase(0, value.find_first_not_of(WHITESPACE));
			buffer.erase(0, line.length() + 1);
		}
		else {
			if (m_invalidRequest.empty()){
				m_invalidRequest = "400 Bad Request\n";
			}
			return 1;
		}
	}
	return 0;
}

std::string Request::getLocationName( void ) {
	std::string path = m_requestPath.substr(1);
	if (m_requestPath.find('.') < m_requestPath.find('?')){
		std::string path = "*" + m_requestPath.substr(m_requestPath.find('.'));
		if (path.find('/') != std::string::npos){
			path.erase(path.find('/'));
		}
		path.insert(0, "/");
		if (m_client.config.locations.find(path) != m_client.config.locations.end()){
			return path;
		}
	}
	return closestMatchingLocation(m_client.config.locations , m_requestPath);
} 

std::string Request::get( std::string str) const {return (m_requestData.find(str) == m_requestData.end() ? "" : m_requestData.find(str)->second);}

bool Request::contains(std::string str) const {return m_requestData.find(str) != m_requestData.end();}

bool	Request::getShowDir( void ) { return m_showDir;};

const std::string& Request::getType( void ) { return m_requestType;}

const std::string& Request::getPath( void ) { return m_requestPath;}

const std::string& Request::getBody( void ) { return m_requestBody;}

const std::string& Request::getInvalidRequest( void ) { return m_invalidRequest;}

const std::string& Request::getQueryString( void ) { return cgi_queryString;}

const std::string& Request::getClientIP( void ) { return m_client.ip;}

const std::string& Request::getPathInfo( void ) {return cgi_pathInfo;}

t_client& Request::getClient( void ){ return m_client;}

void	Request::setPath( std::string newPath ) { m_requestPath = newPath; }

bool	Request::getIsCgi( void ) {return cgi_isCgi;}


