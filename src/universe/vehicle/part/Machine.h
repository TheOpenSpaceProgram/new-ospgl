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
	std::vector<Machine*> get_connected_if(std::function<bool(Machine*)> fnc);

	std::string in_pkg;

public:

	// All Machines share the same lua_state to allow sharing data
	sol::state* lua_state;
	sol::environment env;

	std::shared_ptr<cpptoml::table> init_toml;

	std::unordered_map<std::string, sol::table> interfaces;

	// pre_update is mostly used for input
	void pre_update(double dt);
	void update(double dt);
	void editor_update(double dt);

	void init(sol::state* lua_state, Part* in_part);

	void load_interface(const std::string& name, sol::table n_table);

	std::vector<Machine*> get_all_connected();
	std::vector<Machine*> get_connected_with(const std::vector<std::string>& interfaces);

	sol::table get_interface(const std::string& name);

	// Make sure AssetManager's correct current package is set,
	// otherwise script loading MAY fail!
	Machine(std::shared_ptr<cpptoml::table> init_toml, std::string pkg);
	~Machine();

};


