#pragma once
#include <string>
#include "../util/SerializeUtil.h"
#include "AssetManager.h"

class Config
{
private:

public:
	std::string path;
	std::string in_package;

	std::shared_ptr<cpptoml::table> root;

	template<typename T>
	void read_to(T& target, const std::string& sub_path = "");

	void write_to_file();

};

Config* loadConfig(const std::string& path, const std::string& pkg);

template<typename T>
inline void Config::read_to(T& target, const std::string& sub_path)
{
	std::string old_pkg = assets->get_current_package();

	assets->set_current_package(in_package);
	SerializeUtil::read_to(*root, target, sub_path);
	assets->set_current_package(old_pkg);
}
