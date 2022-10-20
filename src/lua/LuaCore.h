#pragma once
#include "LuaLib.h"

/*
	global:

		We seriously change 'require', 'dofile' and 'loadfile'

		Use 'require' to load other libraries by their name, and 
		other lua files, 'dofile', and 'loadfile' are a bit weird

		- Upon loading LuaCore, all standard lua libraries, except I/O, OS, and Debug are loaded
		  (See: https://www.lua.org/manual/5.3/manual.html#6)
 		- We delete the following functions as they are not appropiate:
 			- 'print', use logger functions instead! These show filename, line, etc...


	'core' table:

	This is the only lua library the programmer actually needs to instantiate as all other libraries
	can be loaded from this one. You can set libraries to load by default, and libraries to block


 	Note: Importing something like require("vehicle") will simply load the symbols, but they wont have a name,
 		you must use local vehicle = require("vehicle") to have access to the symbols. Otherwise you may only
 		access instances created in C++ / other places, which may be appropiate in some cases!

		-> require loads symbols into an unnamed table, and then returns that table!

*/
class LuaCore
{
public:
	
	enum LibraryID
	{
		UNKNOWN,
		LOGGER,			// LuaLogger
		DEBUG_DRAWER,	// LuaDebugDrawer
		GLM,			// LuaGlm
		NOISE,			// LuaNoise
		ASSETS,			// LuaAssets
		BULLET,			// LuaBullet
		TOML,			// LuaToml
		VEHICLE,		// LuaVehicle (Vehicle, Part, Piece, Machine)
		UNIVERSE,		// LuaUniverse (Events, entity creation...)
		FLIGHT_INPUT,	// LuaFlightInput (InputContext)
		GAME_DATABASE,	// LuaGameDatabase
		NANO_VG,		// LuaNanoVG
		PLUMBING,		// LuaPlumbing
		GUI,			// LuaGUI
		IMGUI,			// LuaImGUI
		SCENE,			// LuaScene
		RENDERER,		// LuaRenderer
		MODEL,			// LuaModel
		COUNT,
	};

	static LibraryID name_to_id(const std::string& name);

	LuaLib* libraries[LibraryID::COUNT];

	void load_library(sol::table& to, LibraryID id);

	// pkg is the package the lua file is in, this is used by the 
	// package loader internally ("_pkg" value)
	void load(sol::state& to, const std::string& pkg);

	// This does everything except open the libraries
	void load(sol::table& to, const std::string& pkg);

	LuaCore();
	~LuaCore();
};

extern LuaCore* lua_core;

// Creates the default asset manager, with all 
// asset types loaded 
void create_global_lua_core();

void destroy_global_lua_core();
