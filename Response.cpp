# include "Response.hpp"
# include "Request.hpp"
# include "Error.hpp"
#include "utils.hpp"

Response::Response( Request& request, t_server serv, t_location location ): m_location(location), m_serv(serv) {

	m_responseMap.insert(std::pair<std::string, funcPtr>("GET", &Response::getResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("POST", &Response::postResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("DELETE", &Response::deleteResponse));
	std::map<std::string, funcPtr>::iterator it = m_responseMap.find(request.getType());
	(this->*it->second)(request);
}

/* lists all files in path, excluding "." && ".." */
std::string Response::showDir(std::string path){
	std::stringstream file;
	DIR *dir = opendir(path.c_str());
	if (!dir) throw std::runtime_error(SYS_ERROR("opendir"));
	struct dirent *directory;
	path = path.erase(path.find(m_serv.root), m_serv.root.length());

	file << "<!DOCTYPE html>\n<html>\n	<head>\n		<title>Directory Overview</title>\n	</head>\n	<body>\n		<p>List of files:</p>\n		<dir>\n";

	directory = readdir(dir);
	while ( directory ){
 		if (std::string(directory->d_name) != "." && std::string(directory->d_name) != ".."){
			file << "			<li><a href ='" << path + "/" + std::string(directory->d_name) << "'>" << directory->d_name << "</li>\n";
		}
		directory = readdir(dir);
	}

	file << "		</dir>\n	</body>\n</html>\n";
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
	response << body.str();
	m_response = response.str();
	m_responseSize = m_response.length();
}

void Response::saveGetParam( std::string content ){
	std::stringstream ssContent(content);
	while (std::getline(ssContent, content, '&')) {
		m_UrlParameter[content.substr(0, content.find_first_of('='))] = content.substr(content.find_first_of('=') + 1);
	}
}

void Response::getResponse( Request& request ){
	std::string path = request.getPath();
	std::string resp;
	std::fstream file;
	std::string fileName;

	if (path.find_first_of('?') != std::string::npos){
		saveGetParam(path.substr(path.find_first_of('?') + 1));
		path.erase(path.find_first_of('?'));
	}

	/* set Path for homepage / location to index if available */
	if (path == m_serv.root){
			path.append("/" + m_serv.index);
	}
	else if (path == m_location.path && !m_location.index.empty()){
		path.append("/" + m_location.index);
	}

	/* Pick response Code and Filename */
	if (path == "MethodNotAllowed"){
		resp = "405 Method Not Allowed\n";
		fileName = m_serv.errorPages["405"];
	}
	else if (path == "BadRequest"){
		resp = "400 Bad Request\n";
		fileName = m_serv.errorPages["400"];
	}
	else if (access(path.c_str(), F_OK) == -1){
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
		if (S_ISDIR(statbuf.st_mode)){ // path is directory
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
void Response::postResponse( Request& request ){
	request.setPath(request.getPath() + "?" + request.getBody());
	getResponse(request);
}

void Response::deleteResponse( Request& request ){
	(void)request;
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
