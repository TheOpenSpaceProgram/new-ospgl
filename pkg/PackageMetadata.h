#pragma once
#include <unordered_map>
#include <cpptoml.h>
#include <string>

struct PackageMetadata
{

	std::string id;
	std::string folder;
	std::string name;
	std::string desc;
	std::string author;
	std::string version;
	std::vector<std::string> dependencies;

	std::string pkg_script_path;

	PackageMetadata(std::shared_ptr<cpptoml::table> file)
	{
		name = *file->get_qualified_as<std::string>("name");
		desc = *file->get_qualified_as<std::string>("description");
		author = *file->get_qualified_as<std::string>("author");
		version = *file->get_qualified_as<std::string>("version");
		id = *file->get_qualified_as<std::string>("id");
	
		// We use a default here because it's rarely needed to be changed	
		pkg_script_path = file->get_qualified_as<std::string>("pkg_script").value_or("package.lua");

		// By default the folder is the id
		folder = file->get_qualified_as<std::string>("folder").value_or(id);

		dependencies = std::vector<std::string>();

		auto deps = *file->get_qualified_array_of<std::string>("dependencies");
		for (auto dep : deps)
		{
			dependencies.push_back(dep);
		}
	}

	PackageMetadata()
	{
		// Shall not be used
		name = "Invalid";
	}
};
