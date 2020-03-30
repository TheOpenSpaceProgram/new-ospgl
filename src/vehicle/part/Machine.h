#pragma once
#include <sol.hpp>
#include <cpptoml.h>
#include "../../universe/Universe.h"
#include "../../util/LuaUtil.h"
#include "../../lua/LuaCore.h"

class Vehicle;
class Part;


// Machines implement functionality for parts in lua
// TODO: Think naming of the lua global variables, or 
// even including a little table so they are not really
// global as they may cause naming errors
class Machine
{
private:



	sol::state lua_state;


public:

	std::shared_ptr<cpptoml::table> init_toml;

	void update(double dt);
	//void wire_receive(std::string wire_id, );

	void init(Part* in_part, Universe* in_universe);

	// Make sure AssetManager's correct current package is set,
	// otherwise script loading MAY fail!
	Machine(std::shared_ptr<cpptoml::table> init_toml, std::string pkg);
	~Machine();
};

