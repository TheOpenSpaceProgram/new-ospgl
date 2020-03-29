#pragma once
#include "LuaLib.h"

/*
	global:

		We seriously change 'require', 'dofile' and 'loadfile'

		Use 'require' to load other libraries by their name, and 
		other lua files, 'dofile', and 'loadfile' are a bit weird

		- Upon loading LuaCore, all standard lua libraries, except I/O, OS, and Debug are loaded
		  (See: https://www.lua.org/manual/5.3/manual.html#6)


	'core' table:


	This is the only lua library the programmer actually needs to instantiate as all other libraries
	can be loaded from this one. You can set libraries to load by default, and libraries to block

*/
class LuaCore
{
public:
	
	enum LibraryID
	{
		UNKNOWN,
		LOGGER,		// LuaLogger
		GLM,		// LuaGlm
		NOISE,		// LuaNoise
		ASSETS,		// LuaAssets
		BULLET,		// LuaBullet
		TOML,		// LuaToml
		COUNT
	};

	static LibraryID name_to_id(const std::string& name);

	LuaLib* libraries[LibraryID::COUNT];

	void load_library(sol::table& to, LibraryID id);

	// pkg is the package the lua file is in, this is used by the 
	// package loader internally ("_pkg" value)
	void load(sol::state& to, const std::string& pkg);

	LuaCore();
	~LuaCore();
};

extern LuaCore* lua_core;

// Creates the default asset manager, with all 
// asset types loaded 
void create_global_lua_core();

void destroy_global_lua_core();
