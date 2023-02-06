#include "AssetManager.h"
#include "Shader.h"
#include "Image.h"
#include "Config.h"
#include "BitmapFont.h"
#include "Model.h"
#include "Material.h"
#include "PartPrototype.h"
#include "BuildingPrototype.h"
#include "Cubemap.h"
#include "PhysicalMaterial.h"
#include "AudioClip.h"

#include <game/database/GameDatabase.h>

#include "sol/sol.hpp"

#include <istream>
#include <fstream>
#include <algorithm>

AssetManager* assets;

size_t generated_asset_counter;

AssetManager::AssetManager(const std::string &res_path, const std::string &udata_path)
{
	generated_asset_counter = 0;
	current_package = "core";
	this->res_path = res_path;
	this->udata_path = udata_path;

	preload();

	create_asset_type<Shader>("Shader", load_shader);
	create_asset_type<Image>("Image", load_image);
	create_asset_type<Config>("Config", load_config);
	create_asset_type<BitmapFont>("Bitmap Font", load_bitmap_font);
	create_asset_type<Model>("Model", load_model);
	create_asset_type<Material>("Material", load_material);
	create_asset_type<PartPrototype>("Part Prototype", load_part_prototype);
	create_asset_type<BuildingPrototype>("Building Prototype", load_building_prototype);
	create_asset_type<Cubemap>("Cubemap", load_cubemap);
	create_asset_type<PhysicalMaterial>("Physical Material", load_physical_material);
	create_asset_type<AudioClip>("Audio Clip", load_audio_clip);

	check_packages();
}

bool AssetManager::file_exists(const std::string& path)
{
	return std::filesystem::exists(path);
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

std::vector<uint8_t> AssetManager::load_binary_raw(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	file.unsetf(std::ios::skipws);
	std::streampos file_size;

	file.seekg(0, std::ios::end);
	file_size = file.tellg();
	file.seekg(0, std::ios::beg);

	// reserve capacity
	std::vector<uint8_t> vec;
	vec.reserve(file_size);

	// read the data:
	vec.insert(vec.begin(),
		std::istream_iterator<uint8_t>(file),
		std::istream_iterator<uint8_t>());

	return vec;
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

	while(name[0] == '/')
	{ 
		name = name.substr(1);
	}

	logger->check(is_path_safe(name), "Path {} is unsafe and may access external files", full_path);

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

	return this->res_path + pkg + "/" + name;
}

void AssetManager::check_packages()
{
	for (auto it = packages.begin(); it != packages.end(); it++)
	{
		logger->info("Package '{} {}' ('{}' initialized as '{}')", 
				it->second.metadata.name, it->second.metadata.version, it->second.metadata.id, it->first);

		for (size_t i = 0; i < it->second.metadata.dependencies.size(); i++)
		{
			if (packages.find(it->second.metadata.dependencies[i]) == packages.end())
			{
				logger->warn("  Lacks dependency '{}'", it->second.metadata.dependencies[i]);
			}
		}

	}
}

void AssetManager::preload()
{
	using directory_iterator = std::filesystem::directory_iterator;
	for (const auto& dirEntry : directory_iterator(this->res_path))
	{
		if (dirEntry.is_directory())
		{
			auto path = dirEntry.path();
			std::string as_str = path.string();

			std::replace(as_str.begin(), as_str.end(), '\\', '/');

			std::string sstr = as_str.substr(as_str.find_last_of('/') + 1);

			// Ignore hidden folders
			if(sstr[0] == '.')
			{
				continue;
			}

			PackageMetadata pkg_meta(SerializeUtil::load_file(as_str + "/package.toml"));
			packages[sstr] = Package();
			packages[sstr].was_init = false;
			packages[sstr].metadata = pkg_meta;
		}
	}
	
}

#include "../util/LuaUtil.h"

void AssetManager::load_packages(const std::vector<std::string>& to_load, LuaCore* lua_core, GameDatabase* game_database)
{
	// TODO: Think of a package loading system
	// to allow behaviour similar to ModuleManager
	
	// TODO: Maybe error handling, but a bad package
	// is pretty much a full game crash

	// Pre-init, create all lua files
	for(auto& pkg_pair : packages)
	{
		if(!vector_contains(to_load, pkg_pair.first) || pkg_pair.second.was_init)
		{
			continue;
		}

		std::string pkg_lua_path = res_path + pkg_pair.first + "/" + pkg_pair.second.metadata.pkg_script_path;

		if(file_exists(pkg_lua_path))
		{
			pkg_pair.second.pkg_lua = new sol::state();
			lua_core->load(*pkg_pair.second.pkg_lua, pkg_pair.first);
			bool wrote_error = false;
			LuaUtil::safe_lua(*pkg_pair.second.pkg_lua, load_string_raw(pkg_lua_path), 
				wrote_error, pkg_lua_path);

			if(wrote_error)
			{
				logger->fatal("Package errors are fatal");
			}
		}	
		else
		{
			pkg_pair.second.pkg_lua = nullptr;
		}

		pkg_pair.second.was_init = true;
	}	


	for(auto& pkg_pair : packages)
	{
		sol::state* lua = pkg_pair.second.pkg_lua;

		logger->info("Loading package {}", pkg_pair.first);

		if(lua)
		{
			LuaUtil::call_function((*lua)["load"], game_database);
		}
	}

	game_database->finish_loading();

}

std::pair<sol::state, AssetManager::pfr> AssetManager::load_script(const std::string& pkg, const std::string& path)
{
	sol::state out;

	auto pfr = load_script_to(out, pkg, path);

	return std::make_pair<sol::state, AssetManager::pfr>(std::move(out), std::move(pfr));
}

std::pair<sol::state, AssetManager::pfr> AssetManager::load_script(const std::string& full_path)
{
	auto[pkg, name] = get_package_and_name(full_path, get_current_package());
	return load_script(pkg, name);
}

AssetManager::pfr AssetManager::load_script_to(sol::state& target, const std::string& full_path)
{
	auto[pkg, name] = get_package_and_name(full_path, get_current_package());
	return load_script_to(target, pkg, name);
}

AssetManager::pfr AssetManager::load_script_to(sol::state& target, 
	const std::string& pkg, const std::string& path)
{
	std::string full_path = res_path + pkg + "/" + path;	
	logger->check(file_exists(full_path), "Tried to load an script which does not exist ({})", path);

	lua_core->load(target, pkg);

	// Scripts MUST load, failure to do so will crash the game
	//out.script_file(full_path);
	sol::protected_function_result result = target.safe_script_file(full_path);

	if(!result.valid())
	{
		sol::error err = result;
		logger->fatal("Error while loading script {}:{}:\n{}", pkg, path, err.what());
	}	

	return std::move(result);

}

AssetManager::~AssetManager()
{
	logger->info("Asset manager is being destroyed");
	for(const auto& pkg_pair : packages)
	{
		for(const auto& ast_pair : pkg_pair.second.assets)
		{
			for(const auto& asset_pair : ast_pair.second.second)
			{
				if(asset_pair.second.uses > 0 && !asset_pair.second.dont_unload)
				{
					if(asset_pair.second.dont_unload)
					{
						// TODO: We must unload the asset! Not strictly neccesary but should be good
						logger->debug("Unloading asset '{}' that was set to not_unload", asset_pair.first);
					}
					else
					{
						// Memory leak warning! These assets are never unloaded and must be "stray pointers"
						logger->warn("Asset '{}' still has {} users and is not set to not_unload",
									 asset_pair.first, asset_pair.second.uses);
					}
				}
			}
		}
	}
}

const PackageMetadata& AssetManager::get_package_metadata(const std::string& name) const
{
	auto it = packages.find(name);
	if(it == packages.end())
	{
		logger->fatal("Tried to get metadata for non existing package {}", name);
	}
	return it->second.metadata;
}
std::shared_ptr<cpptoml::table> AssetManager::load_toml(const std::string& full_path, const std::string& def)
{
	std::string resolved = resolve_path(full_path, def);
	// TODO (patching)
	return SerializeUtil::load_file(resolved);
}

std::string AssetManager::get_filename(const std::string &full_path)
{
	size_t bar_pos = full_path.find_last_of('/');
	if(bar_pos == std::string::npos)
	{
		return full_path;
	}
	else
	{
		return full_path.substr(bar_pos + 1);
	}
}

