#include "Request.hpp"
#include "Error.hpp"

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
		m_requestType = "invalid";
		    return ;
	}
    m_requestType = requestLine.at(0);
    m_requestPath = requestLine.at(1);
    m_requestHttpVersion = requestLine.at(2);
    /*
    ########      m_requestHeader noch füllen  ?     ########
    */
    m_requestBody = streamRawRequest.str();
    if (m_requestBody.size() < (unsigned long) m_serv.clientMaxBodySize)
        m_requestBody.resize(m_serv.clientMaxBodySize);
}

unsigned int Request::levenstein(const std::string path, const std::string toMatch){
    if (path.size() == 0)
        return toMatch.size();
    if (toMatch.size() == 0)
        return path.size();
    if (path.at(0) == toMatch.at(0)){
        return levenstein(&path[1], &toMatch[1]);
    }
    return 1 + std::min(
        std::min(
            levenstein(path, toMatch.substr(1)),
            levenstein(path.substr(1), toMatch)
        ),  levenstein(path.substr(1), toMatch.substr(1)));
}

std::string Request::closestMatchingLocation( std::map<std::string, t_location> locMap, std::string path){
    std::vector<std::string> locMapNames;
    for (std::map<std::string, t_location>::iterator it = locMap.begin(); it != locMap.end(); it++){
        locMapNames.push_back(it->first);
    }

    std::vector<unsigned int> diviationIndex;
    for (unsigned long i = 0; i < locMapNames.size(); i++){
        diviationIndex.push_back(levenstein(path, locMapNames[i]));
    }
    int smallestDiviationIndex = std::distance(diviationIndex.begin(), std::min_element(diviationIndex.begin(), diviationIndex.end()));
    return (locMapNames.at(smallestDiviationIndex));
}

std::string Request::getLocationName( void ) { return closestMatchingLocation(m_serv.locations , m_requestPath);} 

Request::~Request(){}

std::string Request::getType( void ) const {return m_requestType;}
std::string Request::getPath( void ) const {return m_requestPath;}
std::string Request::getHttpVersion( void ) const {return m_requestHttpVersion;}
std::string Request::getBody( void ) const {return m_requestBody;}
std::map<std::string, std::string> Request::getRequestHeader( void ) const {return m_requestHeader;}

