#pragma once 
#include <vector>
#include <string>
#include <assets/AssetManager.h>
#include <assets/PhysicalMaterial.h>

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


	std::vector<std::string> parts;
	std::vector<std::string> systems;
	std::vector<std::string> part_categories;
	std::vector<std::string> materials;

	void add_part(const std::string& path, const std::string& pkg);
	void add_part_category(const std::string& path, const std::string& pkg);
	void add_material(const std::string& path, const std::string& pkg);

	GameDatabase();
	~GameDatabase();
};
