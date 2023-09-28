# include "Config.hpp"
# include "utils.hpp"


Config::Config( const Config &cpy){
	m_configFileContent = cpy.m_configFileContent;
	m_servers = cpy.m_servers;
}

std::vector<t_server>   Config::getServerConfig( void ) { return m_servers;}


Config::Config(std::string configFileName ){
	std::fstream infile(configFileName.c_str());
	if (!infile) throw std::runtime_error(SYS_ERROR("open"));

	

	m_funcMap["listen"] = &Config::addListenServer;
	m_funcMap["server_name"] = &Config::addServerNameServer;
	m_funcMap["host"] = &Config::addHostServer;
	m_funcMap["root"] = &Config::addRootServer;
	m_funcMap["index"] = &Config::addIndexServer;
	m_funcMap["error_page"] = &Config::addErrorPageServer;
	m_funcMap["client_max_body_size"] = &Config::addClientMaxBodySize;


	print(Notification, "parsing config file");
	std::stringstream sstream;
	sstream << infile.rdbuf();
	std::string input = sstream.str();
	infile.close();
	while (input.find("server") != std::string::npos){
			addServer(input);
		try {
		}
		catch(std::exception& e){
			std::cerr << e.what() << std::endl;
		};
	}
	print(Notification, "done parsing");

}	

Config::~Config(){}

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
		line = line.substr(line.find_first_not_of(WHITESPACE));
		std::string request = line.substr(0, line.find_first_of(WHITESPACE)); 
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
		throw std::runtime_error(CONFIG_ERROR("invalid autoindex option"));
	}

}

void Config::addIndexLocation(std::string line, t_location& location){
	line = line.substr(line.find("index") + 6);
	line = line.substr(0, line.find_first_of(WHITESPACE));
	if (line.at(line.length() - 1) == '/')
		throw std::runtime_error(CONFIG_ERROR("trailing '/' not allowed for index"));
	if (line.at(0) == '/')
		line.erase(0, 1);
	location.index = line;
}

void Config::addProxyLocation(std::string line, t_location& location) {
	line = line.substr(line.find("proxy_pass") + 11);
	line = line.substr(0, line.find_first_of(WHITESPACE));
	if (line.at(line.length() - 1) == '/')
		throw std::runtime_error(CONFIG_ERROR("trailing '/' not allowed for proxyLocation"));
	if (line.at(0) == '/')
		line.erase(0, 1);
	location.proxyPass = line;
}

void Config::addCgiExtensionLocation(std::string line, t_location& location) {
	line.erase(0, line.find_first_not_of(WHITESPACE));
	line.erase(0, line.find_first_of(WHITESPACE));
	line.erase(0, line.find_first_not_of(WHITESPACE));
	std::stringstream sstream(line);
	while (std::getline(sstream, line, ' ')){
		if (line.at(0) != '.') throw std::runtime_error(CONFIG_ERROR("cgi extension need a '.' at the beginning"));
		location.allowed_cgi_extension.push_back(line);
	}
}

void Config::addMaxBodySizeLocation(std::string line, t_location& location) {
	char *endptr;
	
	double dValue = std::strtod(validateValueFormat(line.substr(line.find_first_not_of(WHITESPACE))).c_str(), &endptr);
	if (static_cast<int>(dValue) != dValue){
		throw std::runtime_error(CONFIG_ERROR("clientMaxBodySize not in int range"));
	}
	if (*endptr || dValue < PORT_MIN || dValue > PORT_MAX){
		throw std::runtime_error(CONFIG_ERROR("clientMaxBodySize out of bounds"));
	}
	location.clientMaxBodySize = (unsigned int)dValue;
}


void Config::addLocation(std::string newLocation, t_server& serv) {
	std::stringstream sstream(newLocation.c_str());
	std::string line;
	t_location location;

	std::getline(sstream, line);
	line = line.substr(line.find_first_of(WHITESPACE));
	line = line.substr(line.find_first_not_of(WHITESPACE));
	line = line.substr(0, line.find_first_of(WHITESPACE));
	if (line != "/" && line.at(line.length() - 1) == '/')
		line.resize(line.length() - 1);
	if (line.at(0) != '/')
		line = "/"+ line;
	location.path = "." + line;
	//init location default values here; (Dont forget error pages & set index to default value or getResponse breaks)

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
		else if (keyWord == "autoindex")
			addAutoindexLocation(line, location);
		else if (keyWord == "index")
			addIndexLocation(line, location);
		else if (keyWord == "proxy_pass")
			addProxyLocation(line, location);
		else if (keyWord == "allowed_cgi_extension"){
			addCgiExtensionLocation(line, location);
		}
		else if (keyWord == "client_max_body_size"){
			addMaxBodySizeLocation(line, location);
		}
	}
	serv.locations[location.path] = location;
}

void Config::fillServerStruct(std::string newServer, t_server& serv){
	std::stringstream sstream(newServer);
	std::string line;


	while (std::getline(sstream, line)){
		if (line.find_first_not_of(WHITESPACE) == std::string::npos)
			continue ;
		if (line.at(line.length() - 1 ) != ';'){
				throw std::runtime_error(CONFIG_ERROR("wrong format ';'"));
		}
		if (line == ";")
			continue ;
		line.resize(line.length() - 1);
		if (line.find_first_not_of(WHITESPACE) == std::string::npos)
			break ;
		line = line.substr(line.find_first_not_of(WHITESPACE));
		if (m_funcMap.find(getFirstWord(line)) != m_funcMap.end()) {
			(this->*m_funcMap[getFirstWord(line)])(line, serv);
		}
		else{
			throw std::runtime_error(CONFIG_ERROR("wrong format"));
		}
	}
}

void Config::addServer(std::string& in){
	std::string newServer = getBlock("server", in);
	t_server serv;

	//init server with default values here;
	
	while (newServer.find("location") != std::string::npos){
		int locationPos = newServer.find_first_not_of(WHITESPACE, newServer.find("location") + std::strlen("location"));
		std::string locName = newServer.substr(locationPos, newServer.find_first_not_of(WHITESPACE));
		std::string newLocation = getBlock("location", newServer);
		addLocation(newLocation, serv);
	}
	newServer = newServer.substr(newServer.find('\n') + 1); 
	newServer.resize(newServer.length() - 1);
	
	fillServerStruct(newServer, serv);
	m_servers.push_back(serv);
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

std::string Config::validateValueFormat(std::string str){
	str = str.substr(str.find_first_of(WHITESPACE));
	str = str.substr(str.find_first_not_of(WHITESPACE));
	if (str.find_first_of(WHITESPACE) != std::string::npos){
		throw std::runtime_error(CONFIG_ERROR("wrong formating"));
	}
	return (str);
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
	if (serv.root.at(serv.root.length() - 1) == '/')
		serv.root.resize(serv.root.length() - 1);
	if (access(serv.root.c_str(), F_OK))
		throw std::runtime_error(CONFIG_ERROR("root not in home directory"));
}

void	Config::addIndexServer(std::string line, t_server& serv){
	
	std::string value = validateValueFormat(line);
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