# include "Config.hpp"

Config::Config(std::string configFileName ){
	m_locationFunctions["index"] = functionWrapper(&Config::addIndex<t_location>);
	m_locationFunctions["allow_methods"] = functionWrapper(&Config::addAllowedMethods<t_location>);
	m_locationFunctions["autoindex"] = functionWrapper(&Config::addAutoIndex<t_location>);
	m_locationFunctions["proxy_pass"] = functionWrapper(&Config::addProxyPass<t_location>);
	m_locationFunctions["allowed_cgi_extension"] = functionWrapper(&Config::addAllowedCgiExtension<t_location>);
	m_locationFunctions["client_max_body_size"] = functionWrapper(&Config::addClientMaxBodySize<t_location>);
	m_locationFunctions["root"] = functionWrapper(&Config::addRoot<t_location>);
	m_locationFunctions["cgi_script"] = functionWrapper(&Config::addCgiName<t_location>);


	m_configFunctions["index"] = functionWrapper(&Config::addIndex<t_config>);
	m_configFunctions["rewrite"] = functionWrapper(&Config::addRedirects<t_config>);
	m_configFunctions["server_name"] = functionWrapper(&Config::addServerName<t_config>);
	m_configFunctions["listen"] = functionWrapper(&Config::addListen<t_config>);
	m_configFunctions["client_max_body_size"] = functionWrapper(&Config::addClientMaxBodySize<t_config>);
	m_configFunctions["root"] = functionWrapper(&Config::addRoot<t_config>);
	m_configFunctions["error_page"] = functionWrapper(&Config::addErrorPages<t_config>);
	
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
	formatPath(locationPath);
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
		try {
			(*m_locationFunctions.at(identifier))(line, location);
		} catch (std::exception &e){
			throw configException(e.what());
		}
	}
	location.path = locationPath;
	serverConfig.locations[locationPath] = location;
}



void Config::addServerConfig(std::string& in) {
	std::string serverBlock = getBlock("server", in);
	t_config serverConfig;
	
	while (serverBlock.find("location") != std::string::npos) {
		addLocation(getBlock("location", serverBlock), serverConfig);
	}
	serverBlock.erase(0, serverBlock.find('\n') + 1);
	serverBlock.resize(serverBlock.find_last_of('}') - 1);

	std::stringstream sstream(serverBlock.c_str());
	std::string line;
	while (std::getline(sstream, line)){
		if (line.find_first_not_of(WHITESPACE) == std::string::npos) {
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
		try {
			(*m_configFunctions.at(identifier))(line, serverConfig);
		} catch (std::exception &e){
			throw configException(e.what());
		}
	}
	m_serverConfig.push_back(serverConfig);
}
