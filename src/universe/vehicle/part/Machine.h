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

	std::vector<Machine*> get_connected_if(std::function<bool(Machine*)> fnc);

public:

	sol::state lua_state;

	std::shared_ptr<cpptoml::table> init_toml;

	std::unordered_map<std::string, sol::table> interfaces;

	// pre_update is mostly used for input
	void pre_update(double dt);
	void update(double dt);
	void editor_update(double dt);

	void init(Part* in_part, Universe* in_universe);

	void load_interface(const std::string& name, sol::table n_table);

	std::vector<Machine*> get_all_connected();
	std::vector<Machine*> get_connected_with(const std::vector<std::string>& interfaces);

	sol::table get_interface(const std::string& name);

	// Make sure AssetManager's correct current package is set,
	// otherwise script loading MAY fail!
	Machine(std::shared_ptr<cpptoml::table> init_toml, std::string pkg);
	~Machine();

};


