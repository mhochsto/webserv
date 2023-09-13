#include "Config.hpp"
# include "Error.hpp"
# 
std::string Config::getBlock( std::string type, std::string& in ){
	if (in.find(type) == std::string::npos) throw std::runtime_error(CONFIG_ERROR("invalid config file"));
	
	std::stack<int> brackets;
	unsigned long i = in.find(type);
	for (; i < in.length(); i++){
		if (in.at(i) == '{'){
			brackets.push(i);
		}
		else if (in.at(i) == '}'){
			brackets.pop();
			if (brackets.empty()){
					break ;
			}
		}
	}
	if (i == in.length() && !brackets.empty()) throw std::runtime_error(CONFIG_ERROR("Open brackets in file"));
	std::string currentScope = in.substr(in.find(type), i + 1 - in.find(type)); // + 1 to get '}'
	in.erase(in.find(type), i + 1 - in.find(type));
	return (currentScope);
}

void Config::addMethodsLocation(std::string line, t_location& location){
	std::string allowedRequests[] = ALLOWED_REQUESTS;

	do {
		line = line.substr(line.find_first_not_of(WHITESPACE)); //skip space front
		std::string request = line.substr(0, line.find_first_of(WHITESPACE)); // get next request
		for (int i = 0; i < ALLOWED_REQUESTS_COUNT; i++){
			if (allowedRequests[i] == request){
				location.allowed_methods.push_back(request);
			}
		}
		line = line.erase(0, line.find(request) + request.length());
	} while (!line.empty());
}

void Config::addAutoindexLocation(std::string line, t_location& location){
	line = line.substr(line.find("autoindex") + 10);
	line = line.substr(0, line.find_first_of(WHITESPACE));
	if (line == "on"){
		location.autoIndex = true;
	}
	else if (line == "off"){
		location.autoIndex = false;
	}
	else {
		std::cout << line << std::endl;
		throw std::runtime_error(CONFIG_ERROR("invalid autoindex option"));
	}

}

void Config::addIndexLocation(std::string line, t_location& location){
	line = line.substr(line.find("index") + 6);
	line = line.substr(0, line.find_first_of(WHITESPACE));
	location.index = line;
}

void Config::addLocation(std::string newLocation, t_server& serv){
	std::stringstream sstream(newLocation.c_str());
	std::string line;
	t_location location;

	std::getline(sstream, line);
	line = line.substr(line.find_first_of(WHITESPACE));
	line = line.substr(line.find_first_not_of(WHITESPACE));
	line = line.substr(0, line.find_first_of(WHITESPACE));
	location.path = line;

	//init location default values here;

	while (std::getline(sstream, line)){
		if (line.at(line.length() - 1) == ';'){
			line.resize(line.length() - 1);
		} else if (line.find_first_not_of(" \t\v\r}") == std::string::npos){
			break ;
		}
		else{
			throw std::runtime_error(CONFIG_ERROR("invalid format"));
		}
		std::string keyWord  = line.substr(line.find_first_not_of(WHITESPACE));
		keyWord = keyWord.substr(0, keyWord.find_first_of(WHITESPACE));
		if (keyWord == "allow_methods")
			addMethodsLocation(line.substr(line.find(keyWord) + keyWord.size()), location);
		if (keyWord == "autoindex")
			addAutoindexLocation(line, location);
		if (keyWord == "index")
			addIndexLocation(line, location);
	}
	serv.locations[location.path] = location;
}

std::string Config::getFirstWord(std::string str){
	str = str.substr(str.find_first_not_of(WHITESPACE));
	return (str.substr(0, str.find_first_of(WHITESPACE)));
}

void Config::fillServerStruct(std::string newServer, t_server& serv){
	std::stringstream sstream(newServer);
	std::string line;

	while (std::getline(sstream, line)){
		if (line.find_first_not_of(WHITESPACE) == std::string::npos) // skip empty line
			continue ;
		if (line.at(line.length() - 1 ) != ';'){
			std::cout << line << std::endl;
				throw std::runtime_error(CONFIG_ERROR("wrong format ';'"));
		}
		line.resize(line.length() - 1);
		if (line.find_first_not_of(WHITESPACE) == std::string::npos)
			break ;
		line = line.substr(line.find_first_not_of(WHITESPACE));
		if ( getFirstWord(line) == "listen"){
			addListenServer(line, serv);
		}
		else if (getFirstWord(line) == "server_name"){
			addServerNameServer(line, serv);
		}
		else if (getFirstWord(line) == "host"){
			addHostServer(line, serv);
		}
		else if (getFirstWord(line) == "root"){
			addRootServer(line, serv);
		}
		else if (getFirstWord(line) == "index"){
			addIndexServer(line, serv);
		}
		else if (getFirstWord(line) == "error_page"){
			//addErrorPageServer(line, serv);
		}
		else if (getFirstWord(line) == "client_max_body_size"){
			addClientMaxBodySize(line, serv);
		}
		else{
			throw std::runtime_error(CONFIG_ERROR("wrong format"));
		}
	}
	exit(1);
	(void)serv;
}

void Config::addServer(std::string& in){
	std::string newServer = getBlock("server", in);
	std::string servName =""; //// Name für map
	t_server serv;

	//init server with default values here;
	
	while (newServer.find("location") != std::string::npos){
		int locationPos = newServer.find_first_not_of(WHITESPACE, newServer.find("location") + std::strlen("location"));
		std::string locName = newServer.substr(locationPos, newServer.find_first_not_of(WHITESPACE));
		std::string newLocation;
		try{
			newLocation = getBlock("location", newServer);
		}
		catch (std::exception& e){
			break ;
		}
		addLocation(newLocation, serv);
	}
	newServer = newServer.substr(newServer.find('\n') + 1); // skip first line
	newServer.resize(newServer.length() - 2); //removes ending bracket
	fillServerStruct(newServer, serv);
	m_servers[servName] = serv;
}

Config::Config(std::string configFileName ){
	std::fstream infile(configFileName.c_str());
	if (!infile) throw std::runtime_error(SYS_ERROR("open"));

	std::stringstream sstream;
	sstream << infile.rdbuf();
	std::string input = sstream.str();
	infile.close();
	while (input.find("server") != std::string::npos){
		try {
			addServer(input);
		}
		catch(std::exception& e){
			std::cerr << e.what() << std::endl;
			break ;
		};
	}

}

Config::~Config(){}

/* returns value as string */
std::string Config::validateValueFormat(std::string str){
	str = str.substr(str.find_first_of(WHITESPACE));
	str = str.substr(str.find_first_not_of(WHITESPACE));
	if (str.find_first_of(WHITESPACE) != std::string::npos){
		throw std::runtime_error(CONFIG_ERROR("wrong formating"));
	}
	return (str);
}

void	Config::addListenServer(std::string line, t_server& serv){
	char *endptr;	
	double dValue = std::strtod(validateValueFormat(line).c_str(), &endptr);
	if (static_cast<int>(dValue) != dValue){
		throw std::runtime_error(CONFIG_ERROR("port not in int range"));
	}
	if (*endptr || dValue < PORT_MIN || dValue > PORT_MAX){
		throw std::runtime_error(CONFIG_ERROR("port invalid"));
	}
	serv.port = static_cast<int>(dValue);
}

void Config::validateServerName(std::string domain){
	std::string domainExtensions[] = ALLOWED_DOMAIN_EXTENSIONS;
	std::string dExtension = domain.substr(domain.find_last_of('.'));
	bool validExtension = false;

	for (int i = 0; i < ALLOWED_DOMAIN_EXTENSIONS_COUNT; i++){
			if (dExtension == domainExtensions[i]){
				validExtension = true;
				break ;
			}
	}
	if (!validExtension){
		throw std::runtime_error(CONFIG_ERROR("invalid server_name extension"));
	}
	std::string str = domain.substr(0, domain.find_last_of('.'));
	if (std::strncmp(domain.c_str(), "www.", 4) || str.find_first_of('.') != std::string::npos){
		return ;
	}
	throw std::runtime_error(CONFIG_ERROR("invalid server_name"));

}

void	Config::addServerNameServer(std::string line, t_server& serv){
	line = line.substr(line.find_first_of(WHITESPACE));
	line = line.substr(line.find_first_not_of(WHITESPACE));

	while(!line.empty()){
		std::string domain = getFirstWord(line);
		line = line.erase(0, line.find(domain) + domain.length());
		validateServerName(domain);
		serv.serverName.insert(domain);
	}
}

void	Config::addClientMaxBodySize(std::string line, t_server& serv){
	char *endptr;
	double dValue = std::strtod(validateValueFormat(line).c_str(), &endptr);
	if (static_cast<int>(dValue) != dValue){
		throw std::runtime_error(CONFIG_ERROR("clientMaxBodySize not in int range"));
	}
	if (*endptr || dValue < PORT_MIN || dValue > PORT_MAX){
		throw std::runtime_error(CONFIG_ERROR("clientMaxBodySize out of bounds"));
	}
	serv.clientMaxBodySize = static_cast<unsigned int> (dValue);
}

void	Config::addHostServer(std::string line, t_server& serv){
	serv.hostname = validateValueFormat(line);
}

void	Config::addRootServer(std::string line, t_server& serv){
	serv.root = validateValueFormat(line);
	if (serv.root.at(0) != '.')
		serv.root = "." + serv.root;
	if (access(serv.root.c_str(), F_OK))
		throw std::runtime_error(CONFIG_ERROR("root not in home directory"));
}

void	Config::addIndexServer(std::string line, t_server& serv){
	
	std::string value = serv.root + validateValueFormat(line);
	if (access(serv.root.c_str(), F_OK))
		throw std::runtime_error(CONFIG_ERROR("index doesn't exist"));
	serv.index = value;
}

void	Config::addErrorPageServer(std::string line, t_server& serv){
	line = line.substr(line.find_first_of(WHITESPACE));
	line = line.substr(line.find_first_not_of(WHITESPACE));

	std::string exitCode = line.substr(0, line.find_first_of(WHITESPACE));
	line = line.substr(line.find(exitCode) + exitCode.length());
	std::string path = validateValueFormat(line);

	if (serv.root.at(serv.root.length() - 1) == '/' && path.at(0) == '/')
		path.erase(0,1);
	if (serv.root.at(serv.root.length() - 1) != '/' && path.at(0) != '/')
		path = "/" + path;
	std::string fullPath = serv.root + path;
	if (access(fullPath.c_str(), F_OK))
		throw std::runtime_error(CONFIG_ERROR("root not in home directory"));
	serv.errorPages[exitCode] = fullPath;
}

