#pragma once 
#include <vector>
#include <string>
#include <assets/AssetManager.h>
#include <assets/PhysicalMaterial.h>
#include <universe/vehicle/material/ChemicalReaction.h>

// Stores different game assets, such as parts, planetary systems, toolbars...
// that may be used by the user and not by code. This is different from assets
// in the fact that the game must know about them to show them to the user, instead
// of an asset being manually selected by code or the user.
// Mods should add their stuff from their package.lua
// This is also where scripts can be attached to different parts of the engine
// Most stuff is only loaded when required, so this stores just the path to the assets
// 
// Note about parts:
// 	Parts that are not added to the GameDatabase can be used, but they will not 
// 	show up on the editor and similar interfaces. 
//
class GameDatabase
{
public:


	// We use this interesting data structure for reactions
	// This way, to find all possible reactions for a combination of materials we need to check
	// only one of them and we save the computation to load time
	std::multimap<std::string, ChemicalReaction> material_to_reactions;
	std::vector<std::string> parts;
	std::vector<std::string> plumbing_machines;
	std::vector<std::string> systems;
	std::vector<std::string> part_categories;
	std::vector<std::string> symmetry_modes;
	std::vector<std::string> reactions;
	std::vector<std::string> materials;

	// id and localized name (ready to display to user)
	std::vector<std::pair<std::string, std::string>> logical_groups;

	std::unordered_map<std::string, std::string> current_locale;

	std::vector<std::string> editor_scripts;

	void add_part(const std::string& path, const std::string& pkg);
	void add_plumbing_machine(const std::string& path, const std::string& pkg);
	void add_part_category(const std::string& path, const std::string& pkg);
	void add_symmetry_mode(const std::string& path, const std::string& pkg);
	void add_material(const std::string& path, const std::string& pkg);
	void add_reaction(const std::string& path, const std::string& pkg);
	void add_logical_group(const std::string& id, const std::string& display_string);

	void finish_loading();

	// Expects a table of tables, in which the first table specifies which locales are provided
	// (the first one being the default if the user locale is not detected) and the following table
	// include an id, followed by the string in the languages as ordered in the first table
	// TODO: We could load all locales to allow quick changes during gameplay, probably unneeded
	void load_locale(const sol::table& locale, const std::string& in_pkg);

	// Gets a string in the current locale
	// REMEMBER: locale ids are automatically prefixed with the mod ID (if it's not already done by the modder)
	// This function will also automatically add the prefix, that's why you must pass pkg. Otherwise, asset
	// manager default is used!
	const std::string& get_string(const std::string& id, const std::string& pkg = "");

	void add_editor_script(const std::string& path, const std::string& pkg);

	GameDatabase();
	~GameDatabase();
};
