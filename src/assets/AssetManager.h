#pragma once
#include <string>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <cassert>
#include <regex>
#include <filesystem>
#include <iostream>

#include <util/Logger.h>
#include <util/SerializeUtil.h>
#include <lua/LuaCore.h>

#include <OSP.h>

#include <PackageMetadata.h>
#include "Asset.h"

// You must allocate the new asset!
// Return nullptr if not possible
template<typename T>
using LoadAssetPtr = T*(*)(ASSET_INFO, const cpptoml::table& config);

template<typename T>
struct AssetHandle;

class GameDatabase;

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
		PackageMetadata metadata;
		std::unordered_map<std::type_index, AssetTypeAndAssets> assets;
		sol::state* pkg_lua;
		bool was_init;
	};
	
	std::unordered_map<std::string, Package> packages;

	template<typename T>
	bool load(const std::string& package, const std::string& name);

	std::string current_package;

public:

	static bool file_exists(const std::string& path);

	// Simply loads a string from given path, no packages or anything
	static std::string load_string_raw(const std::string& path);

	static std::vector<uint8_t> load_binary_raw(const std::string& path);

	// load_string_raw, but package aware
	std::string load_string(const std::string& full_path, const std::string& def = "");

	template<typename T>
	void create_asset_type(const std::string& name, LoadAssetPtr<T> loadPtr, bool preload = false, const std::string& regex = "");

	// If you use this manually make sure you free the asset if it's a heavy-weight resource
	// It crashes if the asset does not exist (or it could not be loaded)
	template<typename T>
	T* get(const std::string& package, const std::string& name, bool use = true);

	// Same as get but returns nullptr if asset could not be loaded
	template<typename T>
	T* get_or_null(const std::string& package, const std::string name, bool use = true);

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

	std::string resolve_path(const std::string& full_path, const std::string& def = "");

	// Checks all dependencies, and logs to console loaded packages
	// and how many assets they have got
	void check_packages();

	// Creates all packages
	void preload();

	// Executes the package init lua file of all packages
	// which have one
	// You must specify which packages to load
	void load_packages(const std::vector<std::string>& to_use, LuaCore* lua_core, GameDatabase* game_database);

	// Checks ../ ~/, and other "escaping" combinations
	// TODO: Check that this is actually secure
	bool is_path_safe(const std::string& path);

	// Scripts are a very special asset as they must be instantiated, you 
	// cannot store a Script as a normal asset
	// * Be careful with the returned "pfr", if you do anything to the state
	// * it will go invalid
	using pfr = sol::protected_function_result;
	std::pair<sol::state, pfr> load_script(const std::string& pkg, const std::string& path);
	std::pair<sol::state, pfr> load_script(const std::string& full_path);
	pfr load_script_to(sol::state& target, const std::string& pkg, const std::string& path);
	pfr load_script_to(sol::state& target, const std::string& full_path);

	void get_config_path(const std::string& pkg);

	std::string res_path;
	std::string udata_path;

	AssetManager(const std::string& res_path, const std::string& udata_path);
	// We do a check to make sure all assets have been released, to prevent leaks!
	~AssetManager();
};

template<typename T>
// If preload is set to true, all files which match the given regex will be loaded
inline void AssetManager::create_asset_type(const std::string& name, LoadAssetPtr<T> loadPtr, bool preload, const std::string& regex)
{
	AssetTypeData tdata;
	tdata.name = name;

	tdata.loadPtr = reinterpret_cast<void*>(loadPtr);

	for (auto it = packages.begin(); it != packages.end(); it++)
	{
		it->second.assets[typeid(T)] = std::make_pair(tdata, std::unordered_map<std::string, Asset>());
	}
}

template<typename T>
inline T* AssetManager::get(const std::string& package, const std::string& name, bool use)
{
	auto pkg = packages.find(package);
	logger->check(pkg != packages.end(), "Invalid package ({}) given", package);

	auto assets = &pkg->second.assets;
	
	auto it = assets->find(typeid(T));
	logger->check(it != assets->end(), "Invalid type given");

	auto item = it->second.second.find(name);

	if (item == it->second.second.end())
	{
		bool result = load<T>(package, name);
		logger->check(result, "Could not load asset ({}:{})", package, name);
		item = it->second.second.find(name);
	}

	if (use)
	{
		item->second.uses++;
	}
	return (T*)(item->second.data);
}

template<typename T>
inline T * AssetManager::get_or_null(const std::string & package, const std::string name, bool use)
{
	auto pkg = packages.find(package);
	logger->check(pkg != packages.end(), "Invalid package ({}) given", package);

	auto assets = &pkg->second.assets;

	auto it = assets->find(typeid(T));
	logger->check(it != assets->end(), "Invalid type given");

	auto item = it->second.second.find(name);

	if (item == it->second.second.end())
	{
		bool result = load<T>(package, name);
		if (!result)
		{
			return nullptr;
		}
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
	logger->check(pkg != packages.end(), "Invalid package ({}) given", package);

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
inline bool AssetManager::load(const std::string& package, const std::string& name)
{
	auto pkg = packages.find(package);
	logger->check(pkg != packages.end(), "Invalid package ({}) given", package);

	auto assets = &pkg->second.assets;

	auto it = assets->find(typeid(T));
	logger->check(it != assets->end(), "Invalid type given");

	LoadAssetPtr<T> fptr = (LoadAssetPtr<T>)(it->second.first.loadPtr);

	std::string full_path = res_path + package + "/" + name;
	std::string full_folder = full_path.substr(0, full_path.find_last_of('/') + 1);

	// Load config
	std::shared_ptr<cpptoml::table> folder_config = cpptoml::make_table();
	std::shared_ptr<cpptoml::table> asset_config = cpptoml::make_table();
	std::shared_ptr<cpptoml::table> cfg = cpptoml::make_table();

	// TODO: Search all folders up in the hierarchy up to package folder
	// to accumulate changes!
	// TODO: Think whether that is actually worth it, or even neccesary


	if (file_exists(full_folder + "folder.toml"))
	{

		folder_config = SerializeUtil::load_file(full_folder + "folder.toml");
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
		if (!cfg->contains(all.first))
		{
			cfg->insert(all.first, all.second);
		} 
	}
	

	std::string old_pkg = get_current_package();
	
	logger->check(file_exists(full_path), "Asset file must exist ({})", full_path);

	set_current_package(package);
	T* ndata = fptr(full_path, name, package, *cfg);
	set_current_package(old_pkg);

	if (ndata == nullptr)
	{
		return false;
	}

	Asset asset;
	asset.uses = 0;
	asset.data = ndata;

	asset.dont_unload = cfg->get_qualified_as<bool>("dont_unload").value_or(false);
	
	it->second.second[name] = asset;
	 
	logger->debug("Loaded asset '{}:{}'", package, name);

	return true;
}

inline bool AssetManager::is_path_safe(const std::string &path)
{
	if(path.find("../") != std::string::npos)
	{
		return false;
	}
	if(path.find("~/") != std::string::npos)
	{
		return false;
	}

	return true;
}


struct AssetPointer
{
	std::string pkg, name;

	std::string to_path() const
	{
		return pkg + ":" + name;
	}

	AssetPointer(const std::string& path, const std::string& def = "")
	{
		std::tie(pkg, name) = osp->assets->get_package_and_name(path, def);
	}

	AssetPointer()
	{
		pkg = "null";
		name = "null";
	}
};



// Automatically frees the asset when done using it
// TODO: We could make this ALWAYS load the asset and implement an AssetPointer which
// only loads it when required? Probably a good idea for consistency
template<typename T>
struct AssetHandle
{
	std::string pkg, name;
	T* data;

public:

	T* get_noconst() const
	{
		logger->check(pkg != "" && name != "", "Tried to get a null asset handle");
		return data;
	}

	const T* get() const
	{
		return get_noconst();
	}

	void unload()
	{
		if (data != nullptr)
		{
			osp->assets->free<T>(pkg, name);
			data = nullptr;
		}
	}

	AssetHandle(const std::string& pkg, const std::string& name)
	{
		this->pkg = pkg;
		this->name = name;

		data = osp->assets->get<T>(pkg, name);
	}

	// This avoids the other constructor from being called when passing pointers
	AssetHandle(const char* pkg, const char* name)
	{
		std::move(AssetHandle(std::string(pkg), std::string(name)));
	}

	AssetHandle(const std::string& path)
	{
		auto[pkg, name] = osp->assets->get_package_and_name(path, osp->assets->get_current_package());
		this->pkg = pkg;
		this->name = name;

		data = osp->assets->get<T>(pkg, name);
	}

	AssetHandle(const AssetPointer& ptr) : AssetHandle(ptr.pkg, ptr.name)
	{
	}

	AssetHandle()
	{
		pkg = "null";
		name = "null";
		data = nullptr;

		// To satisfy std vectors, not really needed
	}

	// We must take the ownership
	AssetHandle(AssetHandle&& b)
	{
		this->pkg = b.pkg;
		this->name = b.name;
		this->data = b.data;

		b.data = nullptr;
		b.pkg = "null";
		b.name = "null";
	}

	AssetHandle<T>& operator=(AssetHandle<T>&& other)
	{
		this->pkg = other.pkg;
		this->name = other.name;
		this->data = other.data;

		other.data = nullptr;
		other.pkg = "null";
		other.name = "null";

		return *this;
	}

	// Returns a new AssetHandle, keeping this one valid
	AssetHandle<T> duplicate() const
	{
		if(data == nullptr)
		{
			return AssetHandle<T>();
		}

		return AssetHandle<T>(pkg, name);
	}

	~AssetHandle()
	{
		unload();
	}

	bool is_null()
	{
		return data == nullptr;
	}

	const T* operator->() const
	{
		return get();
	}

	const T& operator*() const
	{
		return *get();
	}

	bool operator==(const AssetHandle<T>& other) const
	{
		return pkg == other.pkg && name == other.name;
	}

};

namespace std
{
	template<typename T>
	struct hash<AssetHandle<T>>
	{
		size_t operator()(const AssetHandle<T>& k) const
		{
			return std::hash<std::string>()(k.name) ^ std::hash<std::string>()(k.pkg);
		}
	};

}

