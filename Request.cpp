#include "Request.hpp"
#include "Error.hpp"
#include "utils.hpp"

Request::Request(std::string rawRequest, t_server serv): m_serv(serv){
	std::vector<std::string> requestLine;
	std::stringstream streamRawRequest(rawRequest);
	std::stringstream lineStream;
	std::string line;
	std::string word;
	
	/* parse requestLine */
	std::getline(streamRawRequest, line);
	lineStream << line;
	do {
		std::getline(lineStream, word, ' ');
		requestLine.push_back(word);
	} while((word.find_first_of("\r\n")) == std::string::npos && requestLine.size() <= 4); // avoid endless loop

	if (requestLine.size() != 3
		|| !std::strcmp(requestLine.at(2).c_str(), "HTTP/1.1\r\n")){
		m_requestPath = "BadRequest";
			return ;
	}
	m_requestType = requestLine.at(0);
	m_requestPath = serv.root + requestLine.at(1);
	m_requestHttpVersion = requestLine.at(2);
	if (m_requestPath.length() > 1 &&  m_requestPath.at(m_requestPath.length() - 1) == '/')
		m_requestPath.resize(m_requestPath.length() - 1);
	bool valid = false;
	t_location location = m_serv.locations[getLocationName()];
	for (unsigned long i = 0; i < location.allowed_methods.size(); i++){
		if (m_requestType == location.allowed_methods.at(i))
			valid = true;
	}
	if (!valid){
		m_requestPath = "MethodNotAllowed";
		return ;
	}
	/*
	########      m_requestHeader noch füllen  ?     ########
	*/
	const char *findBodyPos = std::strstr(rawRequest.c_str(), "\r\n\r\n");
	findBodyPos ? m_requestBody = findBodyPos + 4 : m_requestBody = "";
	if (m_requestBody.size() > (unsigned long) m_serv.clientMaxBodySize)
		m_requestBody.resize(m_serv.clientMaxBodySize);
}



std::string Request::getLocationName( void ) { return closestMatchingLocation(m_serv.locations , m_requestPath);} 

Request::~Request(){}

void Request::setPath( std::string newPath) {m_requestPath = newPath;}
std::string Request::getType( void ) const {return m_requestType;}
std::string Request::getPath( void ) const {return m_requestPath;}
std::string Request::getHttpVersion( void ) const {return m_requestHttpVersion;}
std::string Request::getBody( void ) const {return m_requestBody;}
std::map<std::string, std::string> Request::getRequestHeader( void ) const {return m_requestHeader;}

