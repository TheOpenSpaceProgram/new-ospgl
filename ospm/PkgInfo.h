#pragma once
#include <string>
#include <cpptoml.h>

// Only stuff needed by OSPM
struct PkgInfo
{
	std::string id;
	std::string folder;
	std::string name;
	std::string version;
	int64_t version_num;

	void load_from_file(const std::string path)
	{
		auto root = cpptoml::parse_file(path);
		read(*root);
	}

	void load_from_memory(char* pkg_data)
	{
		std::stringstream ss = std::stringstream(pkg_data);
		auto root = cpptoml::parser(ss).parse();
		read(*root);
	}

	void read(cpptoml::table& root)
	{
		id = *root.get_as<std::string>("id");
		name = *root.get_as<std::string>("name");
		version = *root.get_as<std::string>("version");
		folder = root.get_as<std::string>("folder").value_or(id);
		version_num = *root.get_as<int64_t>("version_num");
	}

	PkgInfo() {}

};