#include "Request.hpp"


Request::Request(pollfd client, t_server config): m_clientFD(client.fd), m_config(config){
	char buffer[HTTP_HEADER_LIMIT + m_config.clientMaxBodySize];
	std::stringstream buf;
	std::memset(buffer, 0, sizeof(buffer));
	m_readBytes = recv(m_clientFD, buffer, sizeof(buffer), 0);
	if (m_readBytes < 1){
		/* rd == 0 -> delete fd || rd == -1 -> err msg + skip response? */
		return ;
	}
	buf << buffer;
	std::string line;
	std::getline(buf, line);
	if (validateAndSetRequestLine(line)){
		/* Bad Request */
		return ;
	}
	char *chunk = NULL;
	do {
		std::getline(buf, line);
		if (line.empty() && get("Transfer-Encoding") == "chunked\r"){
			int newRd;
			if (!chunk){
				newRd = recv(m_clientFD, &buffer[m_readBytes], sizeof(buffer) - m_readBytes, 0);
				if (newRd < 1){
					return ;
				}
			}
			/* converts hex to decimal */
			std::stringstream hexa;
			std::string tempBuf(&buffer[m_readBytes]);
			unsigned int chunkSize;
			hexa << std::hex << tempBuf.substr(0, tempBuf.find("\r\n")); 
			hexa >> chunkSize; 

			buf.clear();
			buf << tempBuf.substr(tempBuf.find("\r\n"), tempBuf.find("\r\n") + chunkSize);
			m_readBytes += newRd;
			std::getline(buf, line);
			std::cout << line << std::endl;

			char *endptr;
			double chunkSize = std::strtod(line.c_str(), &endptr);
			std::cout << chunkSize << std::endl;
			if (*endptr != '\r' || std::strlen(endptr) != 1){
				/* 400 Bad Request */
				std::cout << "bad return\n";
				return ;
			}
			if (!chunkSize){
				break ;
			}
		}
		else if (line.find_first_of(':') != std::string::npos){
			std::string value = line.substr(line.find_first_of(':') + 1);
			m_requestData[line.substr(0, line.find_first_of(':'))] = value.erase(0, value.find_first_not_of(WHITESPACE));
		} 
		else if (!line.empty()){
			m_requestBody += line;
		}
	} while (true);
	std::cout << "done" << std::endl;
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

	bool valid = false;
	t_location location = m_config.locations[getLocationName()];
	for (unsigned long i = 0; i < location.allowed_methods.size(); i++){
		if (m_requestType == location.allowed_methods.at(i))
			valid = true;
	}
	if (!valid){
		m_requestPath = "MethodNotAllowed";
	}
	return 0;
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
std::string Request::getLocationName( void ) { return closestMatchingLocation(m_config.locations , m_requestPath);} 

Request::~Request(){}

std::string Request::get( std::string str) const {
	if (m_requestData.find(str) == m_requestData.end()){
		return "";
	}
	return m_requestData.find(str)->second;
}

bool Request::contains(std::string str) const {
	return m_requestData.find(str) != m_requestData.end();
}