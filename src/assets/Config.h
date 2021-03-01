#pragma once
#include <string>
#include <util/SerializeUtil.h>
#include "AssetManager.h"
#include "Asset.h"

class Config : public Asset
{
private:

public:

	std::shared_ptr<cpptoml::table> root;

	template<typename T>
	void read_to(T& target, const std::string& sub_path = "");

	void write_to_file();

	Config(ASSET_INFO) : Asset(ASSET_INFO_P) {}

};

Config* load_config(ASSET_INFO, const cpptoml::table& cfg);

template<typename T>
inline void Config::read_to(T& target, const std::string& sub_path)
{
	std::string old_pkg = osp->assets->get_current_package();

	osp->assets->set_current_package(get_asset_pkg());
	SerializeUtil::read_to(*root, target, sub_path);
	osp->assets->set_current_package(old_pkg);
}
