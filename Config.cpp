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

	m_locationFunctions["index"] = Config::FunctionWrapper(&Config::addIndex<t_location>).location();
	m_locationFunctions["allow_methods"] = Config::FunctionWrapper(&Config::addAllowedMethods<t_location>).location();
	m_locationFunctions["autoindex"] = Config::FunctionWrapper(&Config::addAutoIndex<t_location>).location();
	m_locationFunctions["proxy_pass"] = Config::FunctionWrapper(&Config::addProxyPass<t_location>).location();
	m_locationFunctions["allowed_cgi_extension"] = Config::FunctionWrapper(&Config::addAllowedCgiExtension<t_location>).location();
	m_locationFunctions["client_max_body_size"] = Config::FunctionWrapper(&Config::addClientMaxBodySize<t_location>).location();

	m_configFunctions["index"] = Config::FunctionWrapper(&Config::addIndex<t_config>).config();
	m_configFunctions["rewrite"] = Config::FunctionWrapper(&Config::addRedirects<t_config>).config();
	m_configFunctions["server_name"] = Config::FunctionWrapper(&Config::addServerName<t_config>).config();
	m_configFunctions["listen"] = Config::FunctionWrapper(&Config::addListen<t_config>).config();
	m_configFunctions["client_max_body_size"] = Config::FunctionWrapper(&Config::addClientMaxBodySize<t_config>).config();
	m_configFunctions["root"] = Config::FunctionWrapper(&Config::addRoot<t_config>).config();
	m_configFunctions["error_page"] = Config::FunctionWrapper(&Config::addErrorPages<t_config>).config();

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
	t_location location;

	newLocation.resize(newLocation.find_last_of('\n'));
	removeFirstWord(newLocation);
	std::string locationPath = getFirstWord(newLocation);
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
		if (line.at(line.length() - 1) != ';') {
			throw configException("missing ';' in("+ locationPath + "): " + line);
		}
		line.resize(line.length() - 1);
		std::string identifier = getFirstWord(line);
		if (allowedIdentifiersLocation.find(identifier) == allowedIdentifiersLocation.end()){
			throw configException("invalid Identifier in("+ locationPath + "): " + identifier);
		}
		(*m_locationFunctions[identifier])(line, location);
	}
	location.path = locationPath;
	serverConfig.locations[locationPath] = location;
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
		if (line.find_first_of(WHITESPACE) == 0){
			line.erase(0, line.find_first_not_of(WHITESPACE));
		}
		if (allowedIdentifiersServer.find(identifier) == allowedIdentifiersServer.end()){
			throw configException("invalid Identifier in Server Block: " + identifier);
		}
		(*m_configFunctions[identifier])(line, serverConfig);
	}
	m_serverConfig.push_back(serverConfig);
}


std::vector<t_config>   Config::getServerConfig( void ) { return m_serverConfig;}