# include "Response.hpp"
# include "Request.hpp"
# include "Error.hpp"
#include "utils.hpp"

Response::Response( const Request& request, t_server serv, t_location location ): m_location(location), m_serv(serv) {

	m_responseMap.insert(std::pair<std::string, funcPtr>("GET", &Response::getResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("POST", &Response::postResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("DELETE", &Response::deleteResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("BadRequest", &Response::badRequestResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("MethodNotAllowed", &Response::methodNotAllowedResponse));
	std::map<std::string, funcPtr>::iterator it = m_responseMap.find(request.getType());
	(this->*it->second)(request);
}

std::string Response::showDir(std::string path){
	std::stringstream file;
	DIR *dir = opendir(path.c_str());
	if (!dir) throw std::runtime_error(SYS_ERROR("opendir"));
	struct dirent *directory;

	file << "<!DOCTYPE html>\n	<html>\n		<head>\n			<title>Directory Overview</title>\n		</head>\n		<body>\n			<p>List of files:</p>\n			<dir>\n";

	directory = readdir(dir);
	while ( directory ){
		file << "			<li><a href ='" << path + std::string(directory->d_name) << "'car</li>\n";
		directory = readdir(dir);
	}

	file << "			</dir>\n		</body>\n	</html>\n";
	return file.str();
}

void Response::createResponse(std::string rspType, std::string file){
	std::stringstream response;
	std::stringstream body(file);
	response << "HTTP/1.1 " << rspType;
	response << timestamp();
	response << "Server: webserv\nContent-Length: ";
	response << body.str().length();
	response << "\nConnection: Closed\nContent-Type: text/html\n\n";
	response << body;
	m_response = response.str();
	m_responseSize = body.str().length();
}

void Response::getResponse( const Request& request ){
	std::string path = "./" + request.getPath();
	std::string resp;
	std::fstream file;
	std::string fileName;

	if (access(path.c_str(), F_OK) == -1){
		resp = "404 NotFound\n";
		fileName = m_serv.errorPages["404"];
	}
	else if (access(path.c_str(), R_OK) == -1){
		resp = "403 Forbidden\n";
		fileName = m_serv.errorPages["403"];
	}
	else {
		struct stat statbuf;
		std::memset(&statbuf, 0 , sizeof(struct stat));
		if (stat(path.c_str(), &statbuf) == -1) throw std::runtime_error(SYS_ERROR("stat"));
		if (!S_ISDIR(statbuf.st_mode)){ // path is directory
			if (m_location.autoIndex){
				resp = "200 OK\n";
				createResponse(resp, showDir(path));
				return ;
			} else{
				resp = "403 Forbidden\n";
				fileName = m_serv.errorPages["403"];
			}
		}
		else {
			resp = "200 OK\n";
			fileName = path;
		}
	} 
   
	file.open(fileName.c_str());
	if (!file)
		throw std::runtime_error(SYS_ERROR("can't open source file"));
	std::stringstream sstream;
	sstream << file.rdbuf();
	file.close();
	createResponse(resp, sstream.str());
}
void Response::postResponse( const Request& request ){
	(void)request;

}

void Response::deleteResponse( const Request& request ){
	(void)request;
}

void Response::badRequestResponse( const Request& request ){
	(void)request;
	//return 400

}
void Response::methodNotAllowedResponse( const Request& request ){
	(void)request;
	//return 405

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
