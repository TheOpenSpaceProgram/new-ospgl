#pragma once 
#include <vector>
#include <string>

// Stores different game assets, such as parts, planetary systems, toolbars...
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

	void add_part(const std::string& path, const std::string& pkg);

	GameDatabase();
};
