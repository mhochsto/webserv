# include "Response.hpp"
# include "Request.hpp"
# include "Error.hpp"
#include "utils.hpp"

Response::Response( const Request& request, t_server serv, t_location location ): m_location(location), m_serv(serv) {

    m_responseMap.insert(std::pair<std::string, funcPtr>("GET", &Response::getResponse));
    m_responseMap.insert(std::pair<std::string, funcPtr>("POST", &Response::postResponse));
    m_responseMap.insert(std::pair<std::string, funcPtr>("DELETE", &Response::deleteResponse));
    m_responseMap.insert(std::pair<std::string, funcPtr>("invalid", &Response::deleteResponse));

    std::map<std::string, funcPtr>::iterator it = m_responseMap.find(request.getType());
    (this->*it->second)(request);
}


void Response::getResponse( const Request& request ){
    std::stringstream response;
    std::string path = "." + request.getPath();
	std::cout << "root:" << request.getPath() << std::endl;
    if (request.getPath() == "/")
        path = m_serv.root + "/" + m_serv.index;
    else if (request.getPath() == m_location.path)
        path = "./" + m_location.path + "/" + m_location.index;
    response << "HTTP/1.1";
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

	std::fstream file(path.c_str());
	if (!file) throw std::runtime_error(SYS_ERROR("open"));
	response << file.rdbuf();

    m_response = response.str();
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
    //return 400

}

const char *Response::returnResponse( void ) {return (m_response.c_str());}

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
