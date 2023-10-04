# include "Config.hpp"

Config::Config(std::string configFileName ){
	const char *Idenftifers_location[] = ALLOWED_IDENTIFIER_LOCATION;
	for (int i = 0; Idenftifers_location[i]; ++i){
		allowedIdentifiersLocation.insert(Idenftifers_location[i]);
	}
	const char *Idenftifers_server[] = ALLOWED_IDENTIFIER_SERVER;
	for (int i = 0; Idenftifers_server[i]; ++i){
		allowedIdentifiersServer.insert(Idenftifers_server[i]);
	}

	m_functionMap["client_max_body_size"] = &Config::addClientMaxBodySize;
	m_functionMap["autoindex"] = &Config::addAutoIndex;
	m_functionMap["allow_methods"] = &Config::addAllowedMethods;
	m_functionMap["allowed_cgi_extension"] = &Config::addAllowedCgiExtension;
	m_functionMap["index"] = &Config::addIndex;
	m_functionMap["autoindex"] = &Config::addAutoIndex;
	m_functionMap["proxy_pass"] = &Config::addProxyPass;
	m_functionMap["listen"] = &Config::addListen;
	m_functionMap["server_name"] = &Config::addServerName;
	m_functionMap["root"] = &Config::addRoot;

	std::fstream infile(configFileName.c_str());
	if (!infile) throw std::runtime_error(SYS_ERROR("open"));
	std::stringstream sstream;
	sstream << infile.rdbuf();
	infile.close();

	std::string input = sstream.str();
	while (input.find("server") != std::string::npos){
		try {
			addServerConfig(input);
		}
		catch(std::exception& e) {
			std::cerr << e.what() << std::endl;
		};
	}
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

void Config::addLocation(std::string newLocation, t_config& serverConfig) {
	newLocation.resize(newLocation.find_last_of('\n'));
	removeFirstWord(newLocation);
	newLocation.erase(0, newLocation.find_first_not_of(WHITESPACE));
	std::string locationPath = newLocation.substr(0, newLocation.find_first_of(WHITESPACE));
	removeFirstWord(newLocation);
	newLocation.erase(0, newLocation.find_first_not_of(WHITESPACE));
	if (newLocation.substr(0, 2) != "{\n"){
		throw configException("invalid input at location block (" + locationPath + "): " + newLocation);
	}
	newLocation.erase(0, 2);

	std::stringstream sstream(newLocation.c_str());
	std::string line;
	while (std::getline(sstream, line)){
		if (line.find_first_not_of(WHITESPACE) == std::string::npos){
			 continue ;
		}
		if (line.at(line.length() - 1) != ';'){
			throw configException("missing ';' in("+ locationPath + "): " + line);
		}
		line.resize(line.length() - 1);
		std::string identifier = getFirstWord(line);
		removeFirstWord(line);
		if (line.find_first_of(WHITESPACE) == 0){
			line.erase(0, line.find_first_not_of(WHITESPACE));
		}
		
		if (allowedIdentifiersLocation.find(identifier) == allowedIdentifiersLocation.end()){
			throw configException("invalid Identifier in("+ locationPath + "): " + identifier);
		}
		if (serverConfig.locations[locationPath][identifier].size() != 0){
			serverConfig.locations[locationPath][identifier].clear();
		}
		(this->*m_functionMap[identifier])(line, serverConfig.locations[locationPath][identifier]);
	}
}

void Config::addServerConfig(std::string& in){
	std::string serverBlock = getBlock("server", in);
	t_config serverConfig;
	
	while (serverBlock.find("location") != std::string::npos){
		addLocation(getBlock("location", serverBlock), serverConfig);
	}
	serverBlock.erase(0, serverBlock.find('\n') + 1);
	serverBlock.resize(serverBlock.find_last_of('}') - 1);

	std::stringstream sstream(serverBlock.c_str());
	std::string line;
	while (std::getline(sstream, line)){
		if (line.find_first_not_of(WHITESPACE) == std::string::npos){
			 continue ;
		}
		if (line.at(line.length() - 1) != ';'){
			throw configException("missing ';' in Server Block" + line);
		}
		line.resize(line.length() - 1);
		std::string identifier = getFirstWord(line);
		removeFirstWord(line);
		if (line.find_first_of(WHITESPACE) == 0){
			line.erase(0, line.find_first_not_of(WHITESPACE));
		}
		if (allowedIdentifiersServer.find(identifier) == allowedIdentifiersServer.end()){
			throw configException("invalid Identifier in Server Block: " + identifier);
		}
		
		if (identifier == "error_page") {
			addEntry(line, serverConfig.errorPages, false);
		} 
		else if (identifier == "rewrite"){
			addEntry(line, serverConfig.redirects, true);
		}
		else {
			if (serverConfig.Data[identifier].size() != 0){
				serverConfig.Data[identifier].clear();
			}
			(this->*m_functionMap[identifier])(line, serverConfig.Data[identifier]);
		}
	}
	m_serverConfig.push_back(serverConfig);
}

void Config::addIndex(std::string line, std::vector<std::string>& set){
	removeFirstWord(line);
	if (!line.empty()){
		throw configException("invalid index in: [" + line + "]");
	}
	set.push_back(line);
}

void Config::addAllowedMethods(std::string line, std::vector<std::string>& set ){
	const char *allowedRequest[] = ALLOWED_REQUESTS;
	set = convertStringtoVector(line, WHITESPACE);
	if (set.size() == 0){
		throw configException("no methods listed: " + line);
	}
	for(std::vector<std::string>::iterator it = set.begin(); it != set.end(); ++it ) {
		bool valid = false;
		for (unsigned int i = 0; allowedRequest[i]; ++i){
			if (*it == allowedRequest[i]){
				valid = true;
				break ;
			}
		}
		if (valid == false){
			throw configException("invalid Method: [" + *it + "]");
		}
		valid = false;
	}
}

void Config::addProxyPass(std::string line, std::vector<std::string>& set ){
	set.push_back(line);
	removeFirstWord(line);
	if (!line.empty()){
		throw configException("invalid proxy_pass: [" + line + "]");
	}
	formatPath(set.back());
}

void Config::addAllowedCgiExtension(std::string line, std::vector<std::string>& set ){
	set = convertStringtoVector(line, WHITESPACE);
	if (set.size() == 0){
		throw configException("no extensions listed: " + line);
	}
	for (unsigned long i = 0; i < set.size(); ++i){
		if (set.at(i).at(0) != '.'){
			throw configException("invalid cgi extension (needs to start with '.'): " + set.at(i));
		}
	}
}

void Config::addAutoIndex(std::string line, std::vector<std::string>& set ){
	if (line != "off" && line != "on") {
		throw configException("invalid auto_index_option: " + line);
	}
	set.push_back(line);
}

void Config::addClientMaxBodySize(std::string line, std::vector<std::string>& set ){
	char *endptr;
	double dValue = std::strtod(line.c_str(), &endptr);
	if (*endptr || dValue < 0 || dValue > std::numeric_limits<int>::max()){
		throw configException("invalid client_max_body_size in: [" + line + "]");
	}
	set.push_back(line);
}

void 	Config::addListen(std::string line, std::vector<std::string>& set ){
	set.push_back(line);
	removeFirstWord(line);
	if (!line.empty()){
		throw configException("invalid listen: " + line);
	}
	char *endptr;
	double dValue = std::strtod(set.begin()->c_str(), &endptr);
	if (*endptr || dValue < PORT_MIN || dValue > PORT_MAX ){
		throw configException("invalid port: [" + line + "]");
	}
}

void 	Config::addServerName(std::string line, std::vector<std::string>& set ){set = convertStringtoVector(line, WHITESPACE);}

void 	Config::addRoot(std::string line, std::vector<std::string>& set ){
	formatPath(line);
	set.push_back(line);
}

void Config::addEntry(std::string line, vectorMap& Entry, bool urlFormat){

	std::string key = getFirstWord(line);
	removeFirstWord(line);
	std::string value = getFirstWord(line);
	removeFirstWord(line); 
	if (!line.empty() || key.empty() || value.empty()){
		throw configException("invalid errorPage: [" + line + "]");
	}
	
	if (Entry[key].size() != 0){
		Entry[key].clear();
	}

	if (urlFormat){
		formatPath(key);
	}
	formatPath(value);
	Entry[key].push_back(value);
}

std::vector<t_config>   Config::getServerConfig( void ) { return m_serverConfig;}