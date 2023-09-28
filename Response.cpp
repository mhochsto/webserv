# include "Server.hpp"
# include "Response.hpp"
# include "Request.hpp"
# include "Error.hpp"
# include "utils.hpp"
# include "CgiHandler.hpp"

Response::Response(t_client& client, Request& request ): m_client(client) {
	m_responseMap.insert(std::pair<std::string, funcPtr>("GET", &Response::getResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("POST", &Response::postResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("DELETE", &Response::deleteResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("PUT", &Response::putResponse));

	std::map<std::string, funcPtr>::iterator it = m_responseMap.find(request.getType());
	(this->*it->second)(request);
}

/* lists all files in path, excluding "." && ".." */
std::string Response::showDir(std::string path){
	std::stringstream file;
	DIR *dir = opendir(path.c_str());
	if (!dir) throw std::runtime_error(SYS_ERROR("opendir"));
	struct dirent *directory;
	path = path.erase(path.find(m_client.config.root), m_client.config.root.length());

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

void Response::create404Response(void){
	std::fstream file(m_client.config.errorPages["404"].c_str());	
	if (!file){
		throw std::runtime_error(SYS_ERROR("can't open source file"));
	}
	std::stringstream sstream;
	sstream << file.rdbuf();
	file.close();
	createResponse("404 NotFound\n", sstream.str());
}

void Response::putResponse( Request& request ) {
	std::string proxyPassPath = m_client.config.root + "/" + m_client.location.proxyPass;
	if (access(proxyPassPath.c_str(), F_OK) == -1){
		create404Response();
		return ;
	}

	proxyPassPath.append("/" + request.getPath().substr(m_client.location.path.length()));

	std::ofstream file(proxyPassPath.c_str(), std::ios::out | std::ios::trunc);
	if (!file){
		create404Response();
		return ;
	}
	file << m_client.body;
	createResponse("200 OK\n", "");
}

void Response::getResponse( Request& request ){

	std::string path = m_client.config.root + request.getPath();
	std::string resp;
	std::fstream file;
	std::string fileName;
	std::string rawUrlParameter;
	/* save url parameters */
	if (path.find_first_of('?') != std::string::npos){
		rawUrlParameter = path.substr(path.find_first_of('?') + 1);
		saveGetParam(rawUrlParameter);
		path.erase(path.find_first_of('?'));
	}
	/* set Path for homepage / location to index if available */
	if (path == m_client.config.root + "/"){
		path.append(m_client.location.index.empty() ? m_client.config.index : m_client.location.index);
	}

	/* Pick response Code and Filename */
	if (!std::strncmp(path.c_str(), CGI_PATH, std::strlen(CGI_PATH))){
		if (access(path.c_str(), X_OK) == -1){
			resp = "403 Forbidden\n";
			fileName = m_client.config.errorPages["403"];
		}
		else {
			cgiResponse(path, request, rawUrlParameter);
			return ;
		}
	}
	else if (request.getPath() == "MethodNotAllowed"){
		resp = "405 Method Not Allowed\n";
		fileName = m_client.config.errorPages["405"];
	}
	else if (request.getPath() == "BadRequest"){
		resp = "400 Bad Request\n";
		fileName = m_client.config.errorPages["400"];
	}
	else if (access(path.c_str(), F_OK) == -1){
		resp = "404 NotFound\n";
		fileName = m_client.config.errorPages["404"];
	}
	else if (access(path.c_str(), R_OK) == -1){
		resp = "403 Forbidden\n";
		fileName = m_client.config.errorPages["403"];
	}
	else {
		struct stat statbuf;
		std::memset(&statbuf, 0 , sizeof(struct stat));
		if (stat(path.c_str(), &statbuf) == -1) throw std::runtime_error(SYS_ERROR("stat"));
		if (S_ISDIR(statbuf.st_mode)){
			if (m_client.location.autoIndex){
				resp = "200 OK\n";
				createResponse(resp, showDir(path));
				return ;
			} else{
				resp = "403 Forbidden\n";
				fileName = m_client.config.errorPages["403"];
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

//	std::cout <<RED<<  << RESET<<std::endl;
	if (remove(std::string(m_client.config.root + request.getPath()).c_str())  == 0){
		createResponse("204 No Content\n", "");
	}
	else {
		create404Response();
	}
	(void)request;
}

const char *Response::returnResponse( void ) {return (m_response.c_str());}

int  Response::getSize( void ){return (m_responseSize);}

/* STDIN && Env still missing */
void Response::cgiResponse( std::string path, Request& request, std::string rawUrlParameter ){
	CgiHandler cgi(*this, request, m_client.config, path, rawUrlParameter);
	createResponse("200 OK\n", cgi.getOutput());
}
