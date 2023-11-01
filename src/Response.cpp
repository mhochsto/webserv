# include "Server.hpp"
# include "Response.hpp"
# include "Request.hpp"
# include "Error.hpp"
# include "utils.hpp"
# include "CgiHandler.hpp"

Response::Response(t_client& client ): m_client(client) {
	if (!client.request->getInvalidRequest().empty()){
		createErrorResponse(client.request->getInvalidRequest());
		return ;
	}

	m_responseMap.insert(std::pair<std::string, funcPtr>("GET", &Response::getResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("POST", &Response::postResponse));
	m_responseMap.insert(std::pair<std::string, funcPtr>("DELETE", &Response::deleteResponse));

	(this->*m_responseMap.at(client.request->getType()))(client.request);

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

std::string Response::FileType( void ) {
	std::string extension = m_client.request->getPath().substr(1);
	extension.erase(0, m_client.request->getPath().find_last_of('.'));
	if (extension.find_first_of('/') != extension.find_last_of('/')){
		extension.erase(extension.find('/'));
	}
	if (m_client.request->getIsCgi()){
	return "text/html";
	}
	else if (extension == "js"){
			return "application/javascript";
	}
	else if (extension == "css"){
			return "text/css";
	}
	else if (extension == "html"){
			return "text/html";
	}
	return "text/html";
}

void Response::createResponse(std::string rspType, std::string file){
	std::stringstream response;
	std::stringstream body(file);
	response << "HTTP/1.1 " << rspType;
	response << timestamp();
	response << "Server: webserv\nContent-Length: ";
	response << body.str().length();
	response << "\nConnection: keep-alive\n";
	response << "Content-Type: " << FileType() << "\r\n\r\n";
	response << body.str();
	m_response = response.str();
	m_responseSize = m_response.length();
}

/* just for readability */
void Response::createErrorResponse(const std::string& errorCode){
	createResponse(errorCode, createStringFromFile("." + m_client.config.root + m_client.config.errorPages[errorCode.substr(0, 3)]));
}


std::string Response::createStringFromFile(std::string fileName){
	std::fstream file;
	file.open(fileName.c_str());
	if (!file){
		return LAST_RESORT_404;
	}
	std::stringstream sstream;
	sstream << file.rdbuf();
	file.close();
	return sstream.str();
}

void Response::getResponse( Request *request ){
	if (request->getShowDir()){
		createResponse("200 OK\n", showDir(request->getPath()));
		return ;
	}
	createResponse("200 OK\n", createStringFromFile(request->getPath()));
}

std::string Response::postExtension( void ){
	std::string contentType = m_client.request->get("Content-Type");
	if (contentType.empty()){
		return "";
	}
	if (contentType.find("/") != std::string::npos && contentType.find("/") + 2 < contentType.size()){
		return "." + contentType.substr(contentType.find("/") + 1);
	}
	return "";
}

void Response::postResponse( Request *request ){
	if (request->getShowDir()){
		createResponse("200 OK\n", showDir(request->getPath()));
		return ;
	}

	std::string filename = request->getPath().substr(request->getPath().find(m_client.location.path) + m_client.location.path.size() + 1);
	std::cout << RED << filename << std::endl << RESET;
	if (filename.find('/') != std::string::npos){
		createErrorResponse("400 Bad Request\n");
		return ;
	}
	if (!access(request->getPath().c_str(), F_OK)){
		createErrorResponse("409 Conflict\n");
		return ;
	}
	std::fstream newFile;
	newFile.open((request->getPath() + postExtension()).c_str(), std::ios::out);
	if (!newFile.is_open()){
		createErrorResponse("500 Internal Server Error\n");
		return ;
	}
	newFile << request->getBody();
	newFile.close();
	createResponse("200 OK\n", "Data saved successfully\n");
}

void Response::deleteResponse( Request *request ){

	if (remove(std::string(m_client.config.root + request->getPath()).c_str())  == 0){
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
