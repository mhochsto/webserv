#include "Request.hpp"
#include "Error.hpp"


Request::Request(std::string rawRequest){
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
		if (std::find(requestLine.begin(), requestLine.end(), word) == requestLine.end())
			requestLine.push_back(word);
		else
			break ;
	} while(true);

    if (requestLine.size() != 3
        || !std::strcmp(requestLine.at(2).c_str(), "HTTP/1.1\r\n")){
		m_requestType = "invalid";
		    return ;
	}
    m_requestType = requestLine.at(0);
    m_requestPath = requestLine.at(1);
    m_requestHttpVersion = requestLine.at(2);

    /*
    ########      m_requestHeader noch füllen       ########
    */
    m_requestBody = streamRawRequest.str();
    if (m_requestBody.size() < BODYLIMIT)
        m_requestBody.resize(BODYLIMIT);
}

Request::~Request(){}

std::string Request::getType( void ) const {return m_requestType;}
std::string Request::getPath( void ) const {return m_requestPath;}
std::string Request::getHttpVersion( void ) const {return m_requestHttpVersion;}
std::string Request::getBody( void ) const {return m_requestBody;}
std::map<std::string, std::string> Request::getRequestHeader( void ) const {return m_requestHeader;}

