#include "Request.hpp"

Request::Request(t_client& client, std::vector<pollfd>& pollfds): m_client(client), cgi_pollfds(pollfds), cgi_isCgi(false), m_showDir(false), m_isRedirect(false) {

	if (parseHeader() || !m_invalidRequest.empty()){
		return ;
	}
	m_requestBody = client.body;
	setRedirects();
	if (m_isRedirect){
		return ;
	}
	setRoot();
	setPathInfo();
	setIsCgi();
	checkFilePermissions();
	if (!m_invalidRequest.empty()){
		return ;
	}
	checkIfDirectoryShouldBeShown();
	checkBodyLength();
}

Request::~Request(){}

/* bool for testing */
bool Request::verifyHostnameAndResetConfig(std::string requestedHostname){
	t_config *defaultConfig;
	if (requestedHostname.find_first_of('.') != requestedHostname.find_last_of('.')){
		requestedHostname.erase(0, requestedHostname.find_first_of('.') + 1);
	}
	for (std::vector<t_config>::iterator it = m_client.config.sharedConfig.begin(); it != m_client.config.sharedConfig.end(); ++it){
		if (it->def){
			defaultConfig = &*it;
		}
		for (std::vector<std::string>::iterator iter = it->serverName.begin(); iter != it->serverName.end(); ++iter){
			if (*iter == requestedHostname){
				m_client.config = *it;
				return true;
			}
		}	
	}
	if (!m_client.config.def){
		m_client.config = *defaultConfig;
		return true;
	}
	return false;
}

bool Request::checkHostname( void ){
	std::string requestedHostname = m_requestData["Host"];
	if (requestedHostname.empty()){
		m_invalidRequest = "400 Bad Request\n";
		return false;
	}
	requestedHostname = requestedHostname.find(":") == std::string::npos ? requestedHostname : requestedHostname.substr(0, requestedHostname.find(":"));
	verifyHostnameAndResetConfig(requestedHostname);
	return true;
}

void Request::checkBodyLength(void){
	long checkSize = m_client.location.clientMaxBodySize == UNSET ? m_client.config.clientMaxBodySize : m_client.location.clientMaxBodySize;
	if ((long)m_client.body.size() > checkSize) {
		m_invalidRequest = "413 Request Entity Too Large.\n";
	}
}

void	Request::checkIfDirectoryShouldBeShown( void ) {
	 if (m_requestType == "POST"){
	 	return ;
	}
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
			m_requestPath.append("/" + m_client.location.index);
			checkFilePermissions();
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
			m_requestPath = m_requestPath.at(0) != '.' ? m_client.location.cgiScript : m_client.location.cgiScript;
		}
		if (access(m_requestPath.c_str(), F_OK) == -1) {
			m_invalidRequest = "404 File not Found\n";
		}
		else if (access(m_requestPath.c_str(), X_OK) == -1) {
			m_invalidRequest = "403 Forbidden\n";
		}
	} else if (m_requestType == "POST"){
		return ;
	}
	else if (access(m_requestPath.c_str(), F_OK) == -1){
		m_invalidRequest = "404 NotFound\n";
	}
	else if (access(m_requestPath.c_str(), R_OK) == -1){
		m_invalidRequest = "403 Forbidden\n";
	}
}

void Request::setIsCgi(void){

	if ( std::strncmp(m_client.location.path.c_str(), "/*", 2) && std::strncmp(CGI_PATH, m_requestPath.c_str(), std::strlen(CGI_PATH))) {
		return ;
	}
	cgi_isCgi = true;
	if (!std::strncmp(CGI_PATH, m_requestPath.c_str(), std::strlen(CGI_PATH))){
			cgi_scriptName = m_requestPath.substr(m_requestPath.find(CGI_PATH) + std::strlen(CGI_PATH) + 1);
			return ;
	}
	std::string extension = "."  + m_client.location.path.substr(2);
	if (m_client.location.cgiScript.empty()){
		cgi_scriptName = m_requestPath.substr(0, m_requestPath.find(extension));
		cgi_scriptName.erase(0, cgi_scriptName.find_last_of('/') + 1);
	}
	else {
		cgi_scriptName = m_client.location.cgiScript;
	}
	validateExtension(extension, m_client.location);
	if (m_invalidRequest == "405 Method Not Allowed\n") {
		m_invalidRequest.clear();
		validateRequestType(m_client.config.locations[closestMatchingLocation(m_client.config.locations , m_requestPath.substr(1))]);
		validateExtension(extension, m_client.config.locations[closestMatchingLocation(m_client.config.locations , m_requestPath.substr(1))]);
	}
}

void Request::validateExtension(std::string& extension, t_location& location){
	for (std::vector<std::string>::iterator it = location.allowedCgiExtensions.begin(); it != location.allowedCgiExtensions.end(); ++it){
		if (("." + extension) == *it ||   extension == *it){
			return ;
		}
	}
	m_invalidRequest = "403 Forbidden\n";
}

void Request::setPathInfo(void){
	cgi_pathInfo = m_requestPath.substr(1);
	if (cgi_pathInfo.find('.') == std::string::npos) {
		cgi_pathInfo.clear();
		return ;
	}
	cgi_pathInfo.erase(0, cgi_pathInfo.find('.') + 1);
	if (cgi_pathInfo.find('/') != std::string::npos) {
		cgi_pathInfo.erase(0, cgi_pathInfo.find('/'));
		m_requestPath.resize(m_requestPath.find(cgi_pathInfo));
	}
	else {
		cgi_pathInfo.clear();
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
	t_location& location = m_client.config.locations[closestMatchingLocation(m_client.config.locations , m_requestPath)];

	if (location.root.empty()){
		m_requestPath.insert(0, "." + m_client.config.root);
	}
	else {
		m_requestPath.erase(0, location.path.length());
		m_requestPath.insert(0, "." + location.root + (m_requestPath.empty() ? "/" : ""));
	}
}

void Request::setRedirects( void ){
	if (m_client.config.redirects.find(m_requestPath) == m_client.config.redirects.end()){
		return ;
	}
	m_requestPath = m_client.config.redirects[m_requestPath];
	m_isRedirect = true;
}

void Request::validateRequestType(const t_location& location){
	std::vector <std::string> allowedMethods = location.allowedMethods.size() == 0 ? m_client.config.locations["/"].allowedMethods : location.allowedMethods;
	for (unsigned long i = 0; i < allowedMethods.size(); i++) {
		if (m_requestType == allowedMethods.at(i)){
			return ;
		}
	}
	m_invalidRequest = "405 Method Not Allowed\n";
	const char *allowedRequests[] = ALLOWED_REQUESTS;
	for (int i = 0; allowedRequests[i]; ++i){
			if (m_requestType == allowedRequests[i]){
				return ;
			}
	}
	m_invalidRequest = "501 - Method Not Implemented\n";
}

int Request::validateAndSetRequestLine( const std::string& line ) {
	std::stringstream sstream(line);
	std::vector<std::string> vec;
	std::string word;
	while (std::getline(sstream, word, ' ')){
		vec.push_back(word);
		if (vec.size() == 4){
			break;
		}
	}
	if (vec.size() != 3) {
		m_invalidRequest = "400 Bad Request\n";
		return 1;
	}
	m_requestType = vec.at(0);
	m_requestPath = vec.at(1);
	m_requestHttpVersion = vec.at(2);
	if ( !std::strcmp(m_requestHttpVersion.c_str(), "HTTP/1.1\r\n")){
		m_invalidRequest = "505 HTTP Version Not Supported\n";
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
	
	std::string requestLine = line;
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

	checkHostname();

	if (validateAndSetRequestLine(requestLine)){
		return 1;
	}
	return 0;
}

std::string Request::getLocationName( void ) {
	std::string path = m_requestPath.substr(1);
	if (m_requestPath.find('.') < m_requestPath.find('?')){
		path.erase(0, path.find_last_of('.'));
		path.insert(0, "/*");
		if (m_client.config.locations.find(path) != m_client.config.locations.end()){
			return path;
		}
	}
	return closestMatchingLocation(m_client.config.locations , m_requestPath);
} 

std::string Request::get( std::string str) const {return (m_requestData.find(str) == m_requestData.end() ? "" : m_requestData.at(str));}

bool Request::contains(std::string str) const {return m_requestData.find(str) != m_requestData.end();}

bool	Request::getShowDir( void ) { return m_showDir;};

const std::string& Request::getType( void ) { return m_requestType;}

const std::string& Request::getPath( void ) { return m_requestPath;}

const std::string& Request::getBody( void ) { return m_requestBody;}

const std::string& Request::getInvalidRequest( void ) { return m_invalidRequest;}

const std::string& Request::getQueryString( void ) { return cgi_queryString;}

const std::string& Request::getClientIP( void ) { return m_client.ip;}

const std::string& Request::getPathInfo( void ) {return cgi_pathInfo;}

const std::string& Request::getScriptName( void ) { return cgi_scriptName;}

std::vector<pollfd>& Request::getPollfds( void ) { return cgi_pollfds;}

t_client& Request::getClient( void ){ return m_client;}

void	Request::setPath( std::string newPath ) { m_requestPath = newPath; }

void	Request::setInvalidRequest(std::string invalidRequest) { m_invalidRequest = invalidRequest; }

bool	Request::getIsCgi( void ) {return cgi_isCgi;}

bool	Request::getIsRedirect( void ) { return m_isRedirect;}