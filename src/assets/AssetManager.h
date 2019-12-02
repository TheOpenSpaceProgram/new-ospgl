#pragma once
#include <string>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <cassert>
#include <regex>
#include <filesystem>
#include <iostream>
#include "../util/Logger.h"
#include "../util/SerializeUtil.h"


// Pointer to a function which looks like:
// T* loadAsset(const std::string& path)
// You must allocate the new asset!
// Return nullptr if not possible
template<typename T>
using LoadAssetPtr = T*(*)(const std::string&, const std::string&, const cpptoml::table& config);

template<typename T>
struct AssetHandle;

// Organizes all assets into "packages", which are arbitrary
// folders, and "directories", folders inside those packages
// This could allow directly loading files from .zip packages 
// in the future, but for now it uses the normal filesystem
class AssetManager
{
private:
	struct AssetTypeData
	{
		void* loadPtr;
		std::string name;
	};

	struct Asset
	{
		int uses;
		void* data;

		bool preload;
		bool dont_unload;
	};


	// Every asset inside a package
	using PackageAssets = std::unordered_map<std::string, Asset>;
	// Pairs every AssetTypeData with packages
	using AssetTypeAndAssets = std::pair<AssetTypeData, PackageAssets>;


	struct Package
	{

		// Maps every type to a AssetTypeData and packages
		std::unordered_map<std::type_index, AssetTypeAndAssets> assets;

		std::string id;
		std::string folder;
		std::string name;
		std::string desc;
		std::string author;
		std::string version;
		std::vector<std::string> dependencies;

		Package(std::string path)
		{
			std::string package_file_path = path + "/package.toml";
			auto file = SerializeUtil::load_file(package_file_path);
			name = *file->get_qualified_as<std::string>("name");
			desc = *file->get_qualified_as<std::string>("description");
			author = *file->get_qualified_as<std::string>("author");
			version = *file->get_qualified_as<std::string>("version");
			id = *file->get_qualified_as<std::string>("id");

			// By default the folder is the id
			folder = file->get_qualified_as<std::string>("folder").value_or(id);

			dependencies = std::vector<std::string>();

			auto deps = *file->get_qualified_array_of<std::string>("dependencies");
			for (auto dep : deps)
			{
				dependencies.push_back(dep);
			}
		}

		Package()
		{
			// Shall not be used
			name = "Invalid";
		}
	};


	std::unordered_map<std::string, Package> packages;

	template<typename T>
	void load(const std::string& package, const std::string& name);

	std::string current_package;

public:
	
	static bool file_exists(const std::string& path);

	// Simply loads a string from given path, no packages or anything
	static std::string load_string_raw(const std::string& path);

	// load_string_raw, but package aware
	std::string load_string(const std::string& full_path, const std::string& def = "");

	template<typename T>
	void create_asset_type(const std::string& name, LoadAssetPtr<T> loadPtr, bool preload = false, const std::string& regex = "");

	// If you use this manually make sure you free the asset if it's a heavy-weight resource
	template<typename T>
	T* get(const std::string& package, const std::string& name, bool use = true);
	
	// Frees a single use from an asset
	template<typename T>
	void free(const std::string& package, const std::string& name);

	// Valid formats:
	// core:images/navball/navball.png -> "core" "images/navball/navball.png"
	// images/navball/navball.png -> def "images/navball/navball.png"
	std::pair<std::string, std::string> get_package_and_name(const std::string& full_path, const std::string& def);

	// Package used when not any is specified by default
	// Simplifies loading code quite a bit
	void set_current_package(const std::string& pkg);
	std::string get_current_package();

	// If def = "", it will use wathever default path was set by
	// "set_current_package". This is done to make asset loading code simpler
	template<typename T>
	T* get_from_path(const std::string& full_path, const std::string& def = "");

	template<typename T>
	AssetHandle<T> get_handle_from_path(const std::string& full_path, bool load_now = true, const std::string& def = "");

	std::string resolve_path(const std::string& full_path, const std::string& def = "");

	// Checks all dependencies, and logs to console loaded packages
	// and how many assets they have got
	void check_packages();

	// Creates all packages
	void preload();

	AssetManager()
	{
		current_package = "core";
	}
};

template<typename T>
// If preload is set to true, all files which match the given regex will be loaded
inline void AssetManager::create_asset_type(const std::string& name, LoadAssetPtr<T> loadPtr, bool preload, const std::string& regex)
{
	AssetTypeData tdata;
	tdata.name = name;
	tdata.loadPtr = loadPtr;

	for (auto it = packages.begin(); it != packages.end(); it++)
	{
		it->second.assets[typeid(T)] = std::make_pair(tdata, std::unordered_map<std::string, Asset>());
	}
}

template<typename T>
inline T* AssetManager::get(const std::string& package, const std::string& name, bool use)
{
	auto pkg = packages.find(package);
	logger->check(pkg != packages.end(), "Invalid package given");

	auto assets = &pkg->second.assets;
	
	auto it = assets->find(typeid(T));
	logger->check(it != assets->end(), "Invalid type given");

	auto item = it->second.second.find(name);

	if (item == it->second.second.end())
	{
		load<T>(package, name);
		item = it->second.second.find(name);
	}

	if (use)
	{
		item->second.uses++;
	}
	return (T*)(item->second.data);
}

template<typename T>
inline void AssetManager::free(const std::string& package, const std::string& name)
{
	auto pkg = packages.find(package);
	logger->check(pkg != packages.end(), "Invalid package given");

	auto assets = &pkg->second.assets;

	auto it = assets->find(typeid(T));
	logger->check(it != assets->end(), "Invalid type given");

	auto item = it->second.second.find(name);
	item->second.uses--;

	// Check for de-allocation
	if (item->second.uses <= 0 && !item->second.dont_unload)
	{
		logger->debug("Unloaded unused asset '{}:{}'", package, name);
		delete (T*)item->second.data;
		it->second.second.erase(item);
	}
}

template<typename T>
inline T* AssetManager::get_from_path(const std::string& full_path, const std::string& def)
{
	auto[pkg, name] = get_package_and_name(full_path, def);
	return get<T>(pkg, name);
}


template<typename T>
inline void AssetManager::load(const std::string& package, const std::string& name)
{
	auto pkg = packages.find(package);
	logger->check(pkg != packages.end(), "Invalid package given");

	auto assets = &pkg->second.assets;

	auto it = assets->find(typeid(T));
	logger->check(it != assets->end(), "Invalid type given");

	LoadAssetPtr<T> fptr = (LoadAssetPtr<T>)(it->second.first.loadPtr);

	std::string full_path = "./res/" + package + "/" + name;
	std::string full_folder = full_path.substr(0, full_path.find_last_of('/') + 1);

	// Load config
	std::shared_ptr<cpptoml::table> folder_config = cpptoml::make_table();
	std::shared_ptr<cpptoml::table> asset_config = cpptoml::make_table();
	std::shared_ptr<cpptoml::table> cfg = cpptoml::make_table();

	// TODO: Search all folders up in the hierarchy up to package folder
	// to accumulate changes!
	// TODO: Think whether that is actually worth it, or even neccesary


	if (file_exists(full_folder + "assets.toml"))
	{

		folder_config = SerializeUtil::load_file(full_folder + "assets.toml");
	}

	if (file_exists(full_path + ".toml"))
	{
		asset_config = SerializeUtil::load_file(full_path + ".toml");
	}

	// cfg prioritizes asset_config

	for (auto all : *asset_config)
	{
		cfg->insert(all.first, all.second);
	}

	for (auto all : *folder_config)
	{
		if (!cfg->get(all.first))
		{
			cfg->insert(all.first, all.second);
		} 
	}
	

	T* ndata = fptr(full_path, package, *cfg);
	logger->check(ndata != nullptr, "Loaded data must not be null");

	Asset asset;
	asset.uses = 0;
	asset.data = ndata;

	asset.dont_unload = cfg->get_qualified_as<bool>("dont_unload").value_or(false);
	
	it->second.second[name] = asset;
	 
	logger->debug("Loaded asset '{}:{}'", package, name);
}	

extern AssetManager* assets;

// Creates the default asset manager, with all 
// asset types loaded 
void create_global_asset_manager();

void destroy_global_asset_manager();

struct AssetPointer
{
	std::string pkg, name;

	std::string to_path() const
	{
		return pkg + ":" + name;
	}

	AssetPointer(const std::string& path, const std::string& def = "")
	{
		std::tie(pkg, name) = assets->get_package_and_name(path, def);
	}

	AssetPointer()
	{
		pkg = "null";
		name = "null";
	}
};



// Automatically frees the asset when done using it
// It allows holding just a "pointer" to the asset until
// needed (load_now = false)
template<typename T>
struct AssetHandle
{
	std::string pkg, name;
	T* data;

public:

	T* get()
	{
		if (data != nullptr)
		{
			return data;
		}
		else
		{
			data = assets->get<T>(pkg, name);
			return data;
		}
	}

	void unload()
	{
		if (data != nullptr)
		{
			assets->free<T>(pkg, name);
			data = nullptr;
		}
	}

	// Set load_now to false to avoid loading the asset just as the handle is created
	AssetHandle(std::string pkg, std::string name, bool load_now = true)
	{
		this->pkg = pkg;
		this->name = name;

		if (load_now)
		{
			data = assets->get<T>(pkg, name);
		}
		else
		{
			data = nullptr;
		}
	}

	AssetHandle(const AssetPointer& ptr, bool load_now = true) : AssetHandle(ptr.pkg, ptr.name, load_now)
	{
	}

	AssetHandle()
	{
		pkg = "null";
		name = "null";
		data = nullptr;
		
		// To satisfy std vectors, not really needed
	}

	AssetHandle(AssetHandle&& b)
	{
		this->pkg = b.pkg;
		this->name = b.name;
		this->data = b.data;

		b.data = nullptr;
		b.pkg = "null";
		b.name = "null";
	}

	~AssetHandle()
	{
		unload();
	}
};

template<typename T>
inline AssetHandle<T> AssetManager::get_handle_from_path(const std::string & full_path, bool load_now, const std::string & def)
{
	std::string ddef = def == "" ? current_package : def;

	auto[pkg, name] = get_package_and_name(full_path, ddef);

	return AssetHandle<T>(pkg, name, load_now);
}
