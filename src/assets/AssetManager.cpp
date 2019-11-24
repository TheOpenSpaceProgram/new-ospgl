#include "AssetManager.h"
#include "Shader.h"
#include "Image.h"
#include "Config.h"
#include <istream>
#include <fstream>

AssetManager* assets;


void create_global_asset_manager()
{
	assets = new AssetManager();

	assets->preload();

	assets->create_asset_type<Shader>("Shader", loadShader, true, "[.]vs");
	assets->create_asset_type<Image>("Image", loadImage);
	assets->create_asset_type<Config>("Config", loadConfig, true, "[.]toml");

	assets->check_packages();
}

void destroy_global_asset_manager()
{
	delete assets;
}

bool AssetManager::file_exists(const std::string& path)
{
	std::ifstream f(path.c_str());
	return f.good();
}

std::string AssetManager::load_string(const std::string& full_path, const std::string& def)
{
	return load_string_raw(resolve_path(full_path, def));
}

std::string AssetManager::load_string_raw(const std::string& path)
{
	std::ifstream t(path);
	std::string str((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());
	return str;
}

std::pair<std::string, std::string> AssetManager::get_package_and_name(const std::string& full_path, const std::string& def)
{
	std::string ddef = def == "" ? current_package : def;

	std::string package;
	std::string name;

	size_t idx = full_path.find_first_of(':');

	if (idx == std::string::npos)
	{
		package = ddef;
		name = full_path;
	}
	else
	{
		package = full_path.substr(0, idx);
		name = full_path.substr(idx + 1);
	}

	return std::make_pair(package, name);
}

void AssetManager::set_current_package(const std::string& pkg)
{
	this->current_package = pkg;
}

std::string AssetManager::get_current_package()
{
	return current_package;
}

std::string AssetManager::resolve_path(const std::string& full_path, const std::string& def)
{
	auto[pkg, name] = get_package_and_name(full_path, def);

	return "./res/" + pkg + "/" + name;
}

void AssetManager::check_packages()
{
	for (auto it = packages.begin(); it != packages.end(); it++)
	{
		logger->info("Package '{} {}' ({})", it->second.name, it->second.version, it->first);

		for (size_t i = 0; i < it->second.dependencies.size(); i++)
		{
			if (packages.find(it->second.dependencies[i]) == packages.end())
			{
				logger->warn("  Lacks dependency '{}'", it->second.dependencies[i]);
			}
		}

	}
}

void AssetManager::preload()
{
	using directory_iterator = std::filesystem::directory_iterator;
	for (const auto& dirEntry : directory_iterator("./res"))
	{
		if (dirEntry.is_directory())
		{
			auto path = dirEntry.path();
			std::string as_str = path.string();

			std::replace(as_str.begin(), as_str.end(), '\\', '/');

			std::string sstr = as_str.substr(as_str.find_last_of('/') + 1);

			Package pkg(as_str);

			packages[sstr] = pkg;
		}
	}
	
}
