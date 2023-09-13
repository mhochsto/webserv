# include "Response.hpp"
# include "Request.hpp"
#include "Error.hpp"


Response::Response( const Request& request ){

    m_responseMap.insert(std::pair<std::string, funcPtr>("GET", &Response::getResponse));
    m_responseMap.insert(std::pair<std::string, funcPtr>("POST", &Response::postResponse));
    m_responseMap.insert(std::pair<std::string, funcPtr>("DELETE", &Response::deleteResponse));
    m_responseMap.insert(std::pair<std::string, funcPtr>("invalid", &Response::deleteResponse));

//m_responseMap["invalid"] = &Response::deleteResponse;

    std::map<std::string, funcPtr>::iterator it = m_responseMap.find(request.getType());

    (this->*it->second)(request);
}


void Response::getResponse( const Request& request ){
    std::stringstream response;
    std::string path = request.getPath();

	response << "HTTP/1.1";
	if (path == "/")
		path = "./website/index.html";
    else
	    path = "./website/pages" + path;
	if (access(path.c_str(), F_OK) == -1){
	    std::fstream file(FOF_PATH);
	    if (!file) throw std::runtime_error(SYS_ERROR("can't open source file"));
	    response << file.rdbuf();
        m_response = response.str();
        m_responseSize = response.str().length();
        return ;
    }
    
	struct stat statbuf;
	std::memset(&statbuf, 0 , sizeof(struct stat));
	if (stat(path.c_str(), &statbuf) == -1) throw std::runtime_error(SYS_ERROR("stat"));
	if (!S_ISREG(statbuf.st_mode)){
		// not a regular file
		return ;
	} 
	response <<" 200 OK\nContent-Type: text/html\n"; // evtl. anpassen falls es andere files gibt
	std::stringstream length;
	length << statbuf.st_size;
	response << "Content-Length: " << length.str() << "\n\n";

    std::cout << path << std::endl;
	std::fstream file(path.c_str());
	if (!file) throw std::runtime_error(SYS_ERROR("open"));
	response << file.rdbuf();

    m_response = response.str();
    std::cout << m_response;
    m_responseSize = std::atoi(length.str().c_str());
}
void Response::postResponse( const Request& request ){
    (void)request;

}
void Response::deleteResponse( const Request& request ){
    (void)request;
}
void Response::invalidResponse( const Request& request ){
    (void)request;

}

const char *Response::getResponse( void ){return (m_response.c_str());}
int  Response::getSize( void ){return (m_responseSize);}


std::string timestamp(void){
    time_t rawtime;
    struct tm * tm_localTime;
    char buffer[200];

    time (&rawtime);
    tm_localTime = localtime (&rawtime);    
    strftime(buffer, 200,"Date: %a, %d %b %G %T %Z\n",tm_localTime);

    std::string str(buffer);
    return (str.c_str());
}
