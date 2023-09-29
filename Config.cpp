# include "Config.hpp"


std::vector<t_server>   Config::getServerConfig( void ) { return m_servers;}


Config::Config(std::string configFileName ){


/*
	m_serverFunctions[""] = 
	m_serverFunctions[""] = 
	m_serverFunctions[""] = 
	m_serverFunctions[""] = 
	m_serverFunctions[""] = 
	m_serverFunctions[""] = 
*/

	std::fstream infile(configFileName.c_str());
	if (!infile) throw std::runtime_error(SYS_ERROR("open"));

	print(Notification, "parsing config file");
	std::stringstream sstream;
	sstream << infile.rdbuf();
	std::string input = sstream.str();
	infile.close();
	while (input.find("server") != std::string::npos){
		try {
			addServer(input);
		}
		catch(std::exception& e) {
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

std::vector<std::string> Config::convertStringtoVector(std::string str, std::string delimiter){
	std::vector<std::string> v;
	do {
		str.erase(0, str.find_first_not_of(delimiter));
		v.push_back(str.substr(0, str.find_first_of(delimiter)));
		str.erase(0, v.back().length() + 1);
	} while (!str.empty());
	return v;
}

void	Config::removeFirstWord(std::string& str) {
	str.erase(0, str.find_first_not_of(WHITESPACE));
	str.erase(0, str.find_first_of(WHITESPACE));
}

void Config::addtoLocationIfAllowed(std::string& line, std::map< std::string, std::pair<locationFuncPtr, void *> >& locationFunctions){

	std::string identifier = line;
	identifier.erase(0, identifier.find_first_not_of(WHITESPACE));
	identifier.resize(identifier.find_first_of(WHITESPACE));

	if (locationFunctions.find(identifier) == locationFunctions.end()){
		throw configException("invalid Identifier: " + line);
	}
	std::string arguments = line;
	arguments.erase(0, arguments.find(identifier) + identifier.length());
	arguments.erase(0, arguments.find_first_not_of(WHITESPACE));
	(this->*locationFunctions[identifier].first)(arguments, locationFunctions[identifier].second);
}

/* remove one trailing '/' && add one '/' if necessary -> perfect outcome == "/str" */
void Config::formatPath(std::string& str){
	if (str != "/" && str.at(str.length() - 1) == '/')
		str.resize(str.length() - 1);
	if (str.at(0) != '/')
		str = "/"+ str;
}

void Config::addLocation(std::string newLocation, t_server& serv) {
std::cout << RED;
	newLocation.resize(newLocation.find_last_of('\n'));
	std::stringstream sstream(newLocation.c_str());
	std::string line;
	t_location location;
	std::map< std::string, std::pair<locationFuncPtr, void *> > locationFunctions;
	location.autoIndex = LOCATION_AUTO_INDEX;
	location.clientMaxBodySize = -1;

	locationFunctions["index"] = std::make_pair(FunctionWrapper<std::string>(&Config::addIndexLocation).getPtr(), &location.index);
	locationFunctions["allow_methods"] = std::make_pair(FunctionWrapper<std::vector<std::string> >(&Config::addAllowedMethodsLocation).getPtr(), &location.allowed_methods);
	locationFunctions["proxy_pass"] = std::make_pair(FunctionWrapper<std::string>(&Config::addProxyPassLocation).getPtr(), &location.proxyPass);
	locationFunctions["client_max_body_size"] = std::make_pair(FunctionWrapper<ssize_t>(&Config::addClientMaxBodySizeLocation).getPtr(), &location.clientMaxBodySize);
	locationFunctions["allowed_cgi_extension"] = std::make_pair(FunctionWrapper<std::vector<std::string> >(&Config::addAllowedCgiExtensionLocation).getPtr(), &location.allowed_cgi_extension);
	locationFunctions["autoindex"] = std::make_pair(FunctionWrapper<bool>(&Config::addAutoIndexLocation).getPtr(), &location.autoIndex);

	/* Set Location Path; Set '/' pre Path and remove '/' post Path if necessery */
	std::getline(sstream, location.path);
	location.path.erase(0, location.path.find_first_of(WHITESPACE));
	location.path.erase(0, location.path.find_first_not_of(WHITESPACE));

	std::string str = location.path;
	removeFirstWord(str);
	str.erase(0, str.find_first_not_of(WHITESPACE));
	if (str != "{") {
		throw configException("invalid syntax: " + str);
	}

	location.path.resize(location.path.find_first_of(WHITESPACE));
 	formatPath(location.path);

	while (std::getline(sstream, line)){
		if (line.at(line.length() - 1) == ';') {
			line.resize(line.length() - 1);
		}
		else {
			throw configException("invalid formatting: " + line);
		}
		addtoLocationIfAllowed(line, locationFunctions);
	}
	exit(1);
	serv.locations[location.path] = location;
std::cout << RESET;
}

void Config::fillServerStruct(std::string newServer, t_server& serv){
	std::stringstream sstream(newServer);
	std::string line;

	(void)serv;

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
		/*
		if (m_funcMap.find(getFirstWord(line)) != m_funcMap.end()) {
			(this->*m_funcMap[getFirstWord(line)])(line, serv);
		}
		else{
			throw std::runtime_error(CONFIG_ERROR("wrong format"));
		}
		*/
	}
}

void Config::defaultConfigServer(t_server &serv){
	serv.port = SERVER_LISTEN;
	serv.hostname = SERVER_HOST;
	serv.clientMaxBodySize = SERVER_CLIENT_MAX_BODY_SIZE;
	serv.root = SERVER_ROOT;
	serv.index =SERVER_INDEX;
	serv.errorPages["400"] = SERVER_ERROR_PAGE_400;
	serv.errorPages["404"] = SERVER_ERROR_PAGE_404;
	serv.errorPages["405"] = SERVER_ERROR_PAGE_405;
	serv.errorPages["500"] = SERVER_ERROR_PAGE_500;
	serv.errorPages["502"] = SERVER_ERROR_PAGE_502;
	serv.errorPages["503"] = SERVER_ERROR_PAGE_503;
	serv.errorPages["505"] = SERVER_ERROR_PAGE_500;
}

void Config::addServer(std::string& in){
	std::string newServer = getBlock("server", in);
	t_server serv;
	defaultConfigServer(serv);
	
	while (newServer.find("location") != std::string::npos){
		int locationPos = newServer.find_first_not_of(WHITESPACE, newServer.find("location") + std::strlen("location"));
		std::string locName = newServer.substr(locationPos, newServer.find_first_not_of(WHITESPACE));
		std::string newLocation = getBlock("location", newServer);
		addLocation(newLocation, serv);
	}
	newServer.erase(0, newServer.find('\n') + 1);
	newServer.resize(newServer.length() - 10);

	fillServerStruct(newServer, serv);
	m_servers.push_back(serv);
}


/*
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
*/
/*
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
*/