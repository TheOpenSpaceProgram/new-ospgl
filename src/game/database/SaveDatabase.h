#pragma once
#include <cpptoml.h>

// The save database allows scripts to store TOML and other kind of files alongside the
// save-game, providing a means of storing persistent data, but in a sandboxed manner
// To guarantee compatibility of packages, every package gets its own folder to store data,
// while they may read from the folders of other packages, this is not the default behaviour
// TOML files are automatically written to disk, so feel free to modify these persistence files every frame
// or similar as they will be saved only once in a while.
// TODO: Think of a way to allow toml files to be unloaded, as this method will keep all of them loaded.
// for now this is not a real problem as they should be quite lightweight!
class SaveDatabase
{
private:

	std::string save_folder;

	std::unordered_map<std::string, std::shared_ptr<cpptoml::table>> cached;

	std::string form_path(const std::string& pkg, const std::string& path);

public:

	// If the file doesn't exist, it will be created on next flush
	std::shared_ptr<cpptoml::table> get_toml(const std::string& pkg, const std::string& path);

	// Call when saving the game, will write all files to disk, making the changes definitive
	// note that changes are seen in lua before this is called as the files are kept loaded in memory!
	void flush_all();
};
