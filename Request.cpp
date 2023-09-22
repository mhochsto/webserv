#include "Request.hpp"


Request::Request(t_server serv): m_serv(serv){}

int Request::validateRequestLine( void ) {
	std::stringstream sstream(m_requestLine);
	std::vector<std::string> vec;
	std::string word;
	while (std::getline(sstream, word, ' ')){
		vec.push_back(word);
		if (vec.size() == 4) break;
	}
	if (vec.size() != 3 || !std::strcmp(vec.at(2).c_str(), "HTTP/1.1\r\n"))
		return -2;

	m_requestType = vec.at(0);
	m_requestPath = vec.at(1);
	m_requestHttpVersion = vec.at(2);

	bool valid = false;
	t_location location = m_serv.locations[getLocationName()];
	for (unsigned long i = 0; i < location.allowed_methods.size(); i++){
		if (m_requestType == location.allowed_methods.at(i))
			valid = true;
	}
	if (!valid){
		m_requestPath = "MethodNotAllowed";
		return - 2;
	}
	return 1;
}

int Request::receive(int clientFD) {
	int rd;
	char buffer[HTTP_HEADER_LIMIT + m_serv.clientMaxBodySize];
	std::stringstream buf;
	
	rd = recv(clientFD, buffer, sizeof(buffer), 0);
	if (rd < 1)
		return rd;
	buf << buffer;
	
	std::getline(buf, m_requestLine);
	if (!validateRequestLine()){
		return -2;
	}

	std::string line;
	while (std::getline(buf, line)){
		if (line.find_first_of(':') != std::string::npos){
			m_requestData[line.substr(0, line.find_first_of(':'))] = line.substr(line.find_first_of(':') + 1).substr(line.find_first_not_of(WHITESPACE));
		}
		else if (get("Transfer-Encoding") == "chunked"){
			int newRd = recv(clientFD, &buffer[rd], sizeof(buffer) - rd, 0);
			if (newRd < 1)
				return rd;
			buf << &buffer[rd];
			rd += newRd;
			std::getline(buf, line);
			std::cout << line;
		}
	}
	return rd;
}



/*
	std::vector<std::string> requestLine;
	std::stringstream streamRawRequest(rawRequest);
	std::stringstream lineStream;
	std::string line;
	std::string word;
	

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
	while (std::getline(streamRawRequest, line)){
		if (line.find_first_of(':') != std::string::npos){
			m_requestData[line.substr(0, line.find_first_of(':'))] = line.substr(line.find_first_of(':') + 1);
		}
	}

	########      m_requestHeader noch füllen  ?     ########
	const char *findBodyPos = std::strstr(rawRequest.c_str(), "\r\n\r\n");
	findBodyPos ? m_requestBody = findBodyPos + 4 : m_requestBody = "";
	if (m_requestBody.size() > (unsigned long) m_serv.clientMaxBodySize)
		m_requestBody.resize(m_serv.clientMaxBodySize);
}
*/
std::string Request::getLocationName( void ) { return closestMatchingLocation(m_serv.locations , m_requestPath);} 

Request::~Request(){}

std::string Request::get( std::string str) const {
	if (m_requestData.find(str) == m_requestData.end()){
		return "";
	}
	return m_requestData.find(str)->second;
}


bool Request::requestContains(std::string str) const {
	return m_requestData.find(str) != m_requestData.end();}
void Request::setPath( std::string newPath) {m_requestPath = newPath;}
std::string Request::getType( void ) const {return m_requestType;}
std::string Request::getPath( void ) const {return m_requestPath;}
std::map<std::string, std::string>  Request::getData( void ) const {return m_requestData;}
std::string Request::getHttpVersion( void ) const {return m_requestHttpVersion;}
std::string Request::getBody( void ) const {return m_requestBody;}
std::string Request::getRequestHttpVersion( void ) const { return m_requestHttpVersion;}
std::map<std::string, std::string> Request::getRequestHeader( void ) const {return m_requestHeader;}

