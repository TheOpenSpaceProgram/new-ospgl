#pragma once
#include <string>

// Macros to shorten the constructors a little bit
#define ASSET_INFO const std::string& path, const std::string& name, const std::string& pkg
#define ASSET_INFO_P path, name, pkg
// For generated assets
#define GENERATED_ASSET_INFO "", "null", "generated_asset"

// Base class for an asset, contains basic information and hashing
class Asset
{
private:
	std::string asset_pkg;
	std::string asset_name;
	std::string asset_id;
	std::string asset_resolved_path;

public:

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
