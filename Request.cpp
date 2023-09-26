#include "Request.hpp"


Request::Request(t_client& client): m_config(client.config), m_client(client) {
	char c_buffer[HTTP_HEADER_LIMIT + m_config.clientMaxBodySize];
	std::stringstream sstreamBuffer;
	std::memset(c_buffer, 0, sizeof(c_buffer));
	m_readBytes = recv(client.fd, c_buffer, sizeof(c_buffer), 0);
	if (m_readBytes == - 1){
		return ;
	}
	else if (m_readBytes == 0){
		m_requestPath = "BadRequest";
		return ;
	}
	std::string sBuffer(c_buffer);

	ErrCode status = parseHeader(sBuffer);
	if (status != Ok){
		return ;
	}
	if (m_requestData.find("Transfer-Encoding") != m_requestData.end() && m_requestData["Transfer-Encoding"] == "chunked"){
		status = recvChunks(m_requestBody);
		if (status != Ok){
		return ;
		}
	}
	else {
		m_requestBody = sBuffer;
	}
	if (m_requestBody.size() > (unsigned long) m_config.clientMaxBodySize){
		m_requestPath = "RequestEntityTooLarge";
	}
}

Request::~Request(){}


ErrCode	Request::recvChunks(std::string& body){
	char buffer[m_config.clientMaxBodySize];
	do {
		std::memset(buffer, 0, sizeof(buffer));
		ssize_t rd = recv(m_clientFD, buffer, sizeof(buffer), 0);
		if (rd == -1){
			return Error;
		}
		else if (rd == 0){
			return BadRequest;
		}
		std::string sBuffer(buffer);
		std::string chunkSizeAsString = sBuffer.substr(0, sBuffer.find("\r\n"));	
		char *end = NULL;
		long chunkSizeLong = std::strtol(chunkSizeAsString.c_str(), &end, 16);
		if (*end){
			m_requestPath = "BadRequest";
			return BadRequest;
		}
		if (chunkSizeLong == 0){
			return Ok;
		}
		sBuffer.erase(0, chunkSizeAsString.length() + 2); // "+2 for \r\n"
		if ((long)sBuffer.length() - 2 != chunkSizeLong){
			m_requestPath = "BadRequest";
			return BadRequest;
		}
		body.append(sBuffer);	
	} while (true);
	return Ok;
}

ErrCode Request::parseHeader( std::string& buffer ){
	std::stringstream sstreamBuffer(buffer);
	std::string line;
	std::getline(sstreamBuffer, line);
		if (validateAndSetRequestLine(line)){
		return BadRequest;
	}
	buffer.erase(0, line.length() + 1);
	while (std::getline(sstreamBuffer, line)){
		if (line.find_first_of(':') != std::string::npos){
			std::string value = line.substr(line.find_first_of(':') + 1);
			m_requestData[line.substr(0, line.find_first_of(':'))] = value.erase(0, value.find_first_not_of(WHITESPACE));
			buffer.erase(0, line.length() + 1);
		}
		else {
			return Ok;
		}
	}
	return Ok;
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
	/*todo : location path is wrong - fill location in client struct */
	for (unsigned long i = 0; i < location.allowed_methods.size(); i++){
		if (m_requestType == location.allowed_methods.at(i))
			valid = true;
	}
	if (!valid){
		std::cout <<"reached\n";
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