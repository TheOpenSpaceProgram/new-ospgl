#include "Config.h"

Config* load_config(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& ccfg)
{
	logger->info("Loading config from file: '{}'", path);

	Config* cfg = new Config();
	cfg->root = SerializeUtil::load_file(path);
	cfg->path = path;
	cfg->in_package = pkg;

	return cfg;
}

void Config::write_to_file()
{
	SerializeUtil::write_to_file(*root, path);
}
