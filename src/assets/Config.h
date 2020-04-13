#pragma once
#include <string>
#include <util/SerializeUtil.h>
#include "AssetManager.h"

class Config
{
private:

public:
	// Full resolved path to the config
	std::string real_path;
	std::string path;
	std::string pkg;

	std::shared_ptr<cpptoml::table> root;

	template<typename T>
	void read_to(T& target, const std::string& sub_path = "");

	void write_to_file();

};

Config* load_config(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg);

template<typename T>
inline void Config::read_to(T& target, const std::string& sub_path)
{
	std::string old_pkg = assets->get_current_package();

	assets->set_current_package(pkg);
	SerializeUtil::read_to(*root, target, sub_path);
	assets->set_current_package(old_pkg);
}
