# include "Server.hpp"
# include "Response.hpp"
# include "Request.hpp"
# include "Error.hpp"
# include "utils.hpp"
# include "CgiHandler.hpp"

Response::Response(t_client& client, Request& request ): m_client(client), m_request(request) {
	if (!request.getInvalidRequest().empty()){
		createErrorResponse(request.getInvalidRequest());
		return ;
	}
	if (request.getIsCgi()){
		executeCGI();
		return ;
	}
	m_responseMap.insert(std::pair<std::string, funcPtr>("GET", &Response::getResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("POST", &Response::postResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("DELETE", &Response::deleteResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("PUT", &Response::putResponse));

	std::map<std::string, funcPtr>::iterator it = m_responseMap.find(request.getType());
	(this->*it->second)(request);
}

void Response::executeCGI(void){
	CgiHandler cgi(m_request);
	createResponse("200 Ok\n", cgi.getOutput());
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
	response << "\nConnection: keep-alive\nContent-Type: text/html\n\n";
	response << body.str();
	m_response = response.str();
	m_responseSize = m_response.length();
}

/* this is just for readability */
void Response::createErrorResponse(const std::string& errorCode){
	createResponse(errorCode, createStringFromFile("." + m_client.config.root + m_client.config.errorPages[errorCode.substr(0, 3)]));
}

void Response::putResponse( Request& request ) {
	std::string proxyPassPath = m_client.config.root + "/" + m_client.location.proxyPass;
	if (access(proxyPassPath.c_str(), F_OK) == -1){
		createErrorResponse("400 File not Found\n");
		return ;
	}
	proxyPassPath.append("/" + request.getPath().substr(m_client.location.path.length()));

	std::ofstream file(proxyPassPath.c_str(), std::ios::out | std::ios::trunc);
	if (!file){
		createErrorResponse("400 File not Found\n");
		return ;
	}
	file << m_client.body;
	createResponse("200 OK\n", "");
}

std::string Response::createStringFromFile(std::string fileName){
	std::fstream file;
	file.open(fileName.c_str());
	if (!file)
		throw std::runtime_error(SYS_ERROR("can't open source file"));
	std::stringstream sstream;
	sstream << file.rdbuf();
	file.close();
	return sstream.str();
}

void Response::getResponse( Request& request ){
	if (request.getShowDir()){
		createResponse("200 OK\n", showDir(request.getPath()));
		return ;
	}
	createResponse("200 OK\n", createStringFromFile(request.getPath()));
}

void Response::postResponse( Request& request ){
	//request.setPath(request.getPath() + "?" + request.getBody());
	createResponse("200 Ok\n", request.getClient().body);
}

void Response::deleteResponse( Request& request ){

	if (remove(std::string(m_client.config.root + request.getPath()).c_str())  == 0){
		createResponse("204 No Content\n", "");
	}
	else {
		createErrorResponse("400 File not Found\n");
	}
}

const char *Response::returnResponse( void ) const {return (m_response.c_str());}

int  Response::getSize( void ) const {return (m_responseSize);}


std::ostream    &operator<<(std::ostream &os, const Response &rhs) {
	os << "What's in the response Class!" << std::endl;
	os << "\tResponse.size():" << rhs.getSize() << std::endl;
	os << "\tResponse.returnResponce():" << rhs.returnResponse() << std::endl;
	return (os);
}
