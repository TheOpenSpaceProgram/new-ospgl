#pragma once
#include <sol.hpp>
#include <cpptoml.h>
#include <universe/Universe.h>
#include <util/LuaUtil.h>
#include <lua/LuaCore.h>

class Vehicle;
class Part;


// Machines implement functionality for parts in lua
// TODO: Think naming of the lua global variables, or 
// even including a little table so they are not really
// global as they may cause naming errors
class Machine
{
private:



	Part* in_part;
	bool is_defining_ports;

	struct PortDefinition
	{
		std::string name;
		std::string type;
		bool output;

		// Only on input ports
		sol::safe_function callback;

	};

	std::vector<PortDefinition> new_ports;

public:

	sol::state lua_state;

	std::shared_ptr<cpptoml::table> init_toml;

	// pre_update is mostly used for input
	void pre_update(double dt);
	void update(double dt);
	void editor_update(double dt);

	void init(Part* in_part, Universe* in_universe);

	// Make sure AssetManager's correct current package is set,
	// otherwise script loading MAY fail!
	Machine(std::shared_ptr<cpptoml::table> init_toml, std::string pkg);
	~Machine();

};

