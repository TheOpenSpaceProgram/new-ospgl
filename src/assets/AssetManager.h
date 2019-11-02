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


// Pointer to a function which looks like:
// T* loadAsset(const std::string& path)
// You must allocate the new asset!
// Return nullptr if not possible
template<typename T>
using LoadAssetPtr = T*(*)(const std::string&);

class AssetManager
{
private:
	struct AssetTypeData
	{
		std::string prefix;
		std::string extension;
		void* loadPtr;
	};

	std::unordered_map<std::type_index, std::pair<AssetTypeData, std::unordered_map<std::string, void*>>> assets;
		
public:

	// Low level file accesors
	static std::string loadString(const std::string& path);

	template<typename T>
	void createAssetType(LoadAssetPtr<T> loadPtr, std::string prefix, std::string extension, bool preload = false);

	template<typename T>
	T* get(const std::string& name);

	template<typename T>
	T* load(const std::string& name);
};

template<typename T>
// If preload is set to true, all files with extension 'extension' will be loaded
inline void AssetManager::createAssetType(LoadAssetPtr<T> loadPtr, std::string prefix, std::string extension, bool preload)
{
	logger->debug("Creating new asset type with prefix '{}' and extension '{}'", prefix, extension);

	AssetTypeData tdata;
	tdata.prefix = prefix;
	tdata.loadPtr = loadPtr;
	tdata.extension = extension;
	assets[typeid(T)] = std::make_pair(tdata, std::unordered_map<std::string, void*>());

	if (preload)
	{
		using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
		for (const auto& dirEntry : recursive_directory_iterator(prefix))
		{
			if (dirEntry.is_regular_file())
			{
				auto path = dirEntry.path();
				std::string as_str = path.string();

				std::replace(as_str.begin(), as_str.end(), '\\', '/');

				if (std::regex_match(as_str, std::regex(".+\\." + extension)))
				{
					// Remove the path and extension
					std::string id_str = as_str.substr(prefix.size());
					id_str = id_str.substr(0, id_str.find_last_of('.'));

					load<T>(id_str);
				}
			}
		}
	}
}

template<typename T>
inline T* AssetManager::get(const std::string& name)
{
	auto it0 = assets.find(typeid(T));
	// Make sure the asset type exists
	logger->check(it0 != assets.end(), "Asset type must exist");	

	auto map = &it0->second.second;
	auto it1 = map->find(name);

	if (it1 == map->end())
	{
		// Load the file
		return load<T>(name);
	}
	else
	{
		return (T*)(it1->second);
	}
}

template<typename T>
inline T* AssetManager::load(const std::string& name)
{
	auto it0 = assets.find(typeid(T));

	const std::string fullPath = it0->second.first.prefix + name + '.' + it0->second.first.extension;

	logger->debug("Loading '{}' as '{}'", fullPath, name);

	LoadAssetPtr<T> ptr = (LoadAssetPtr<T>)it0->second.first.loadPtr;
	T* nData = ptr(fullPath);

	// Make sure something loaded
	logger->check(nData != nullptr, "Loaded data must not be null");

	auto map = &it0->second.second;

	(*map)[name] = nData;

	return nData;
}

extern AssetManager* assets;

// Creates the default asset manager, with all 
// asset types loaded
void create_global_asset_manager();

void destroy_global_asset_manager();

