#include "Request.hpp"


Request::Request(t_client& client): m_config(client.config), m_client(client) {
	
	if (parseHeader(client.header)){
		m_requestPath = "BadRequest";
		m_requestType = "GET";
	}

}

Request::~Request(){}

int Request::parseHeader( std::string& buffer ){
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
			return 0;
		}
	}
	return 0;
}

int Request::validateAndSetRequestLine( std::string line ) {
	std::stringstream sstream(line);
	std::vector<std::string> vec;
	std::string word;
	while (std::getline(sstream, word, ' ')){
		vec.push_back(word);
		if (vec.size() == 4) break;
	}
	if (vec.size() != 3 || !std::strcmp(vec.at(2).c_str(), "HTTP/1.1\r\n"))
		return 1;
	m_requestType = vec.at(0);
	m_requestPath = vec.at(1);
	m_requestHttpVersion = vec.at(2);
	m_client.location = m_config.locations[getLocationName()];
	bool valid = false;
	for (unsigned long i = 0; i < m_client.location.allowed_methods.size(); i++){
		if (m_requestType == m_client.location.allowed_methods.at(i)){
			valid = true;
			break ;
		}
	}
	if (!valid){
		m_requestPath = "MethodNotAllowed";
	}
	return 0;
}

std::string Request::getLocationName( void ) { return closestMatchingLocation(m_config.locations , m_requestPath);} 

std::string Request::get( std::string str) const {
	if (m_requestData.find(str) == m_requestData.end()){
		return "";
	}
	return m_requestData.find(str)->second;
}

bool Request::contains(std::string str) const {
	return m_requestData.find(str) != m_requestData.end();
}

std::string Request::getType( void ) { return m_requestType;}

std::string Request::getPath( void ) { return m_requestPath;}

std::string Request::getBody( void ) { return m_requestBody;}

void		Request::setPath( std::string newPath ) { m_requestPath = newPath; }

std::string Request::getClientIP( void ) { return m_client.ip;}