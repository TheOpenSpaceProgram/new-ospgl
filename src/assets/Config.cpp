#include "Config.h"

Config* load_config(ASSET_INFO, const cpptoml::table& ccfg)
{
	logger->info("Loading config from file: '{}'", path);

	Config* cfg = new Config(ASSET_INFO_P);
	cfg->root = SerializeUtil::load_file(path);

	return cfg;
}

void Config::write_to_file() const
{
	SerializeUtil::write_to_file(*root, get_asset_resolved_path());
}
