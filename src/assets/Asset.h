#pragma once
#include <string>

// Macros to shorten the constructors a little bit
#define ASSET_INFO const std::string& path, const std::string& name, const std::string& pkg
#define ASSET_INFO_P path, name, pkg
// For generated assets this uses a global static function
#define GENERATED_ASSET_INFO "", Asset::get_generated_package_name(), "generated_assets"

// Defined in AssetManager.cpp (It's a global variable)
extern size_t generated_asset_counter;

// Base class for an asset, contains basic information and hashing
class Asset
{
private:
	std::string asset_pkg;
	std::string asset_name;
	std::string asset_id;
	std::string asset_resolved_path;

public:

	static std::string get_generated_package_name()
	{
		generated_asset_counter++;
		return "asset_" + std::to_string(generated_asset_counter);
	}

	const std::string& get_asset_id() const
	{
		return asset_id;
	}

	const std::string& get_asset_pkg() const
	{
		return asset_pkg;
	}

	const std::string& get_asset_name() const
	{
		return asset_name;
	}

	const std::string& get_asset_resolved_path() const
	{
		return asset_resolved_path;
	}

	Asset(ASSET_INFO)
	{
		asset_name = name;
		asset_pkg = pkg;
		asset_id = pkg + ":" + name;
		asset_resolved_path = path;
	}

};
