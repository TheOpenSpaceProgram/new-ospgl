#pragma once
#include <sol.hpp>
#include <cpptoml.h>
#include <universe/Universe.h>
#include <util/LuaUtil.h>
#include <lua/LuaCore.h>

#include "../wire/Port.h"

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

	std::vector<Port*> ports;
	// Only outputs because inputs cannot be read unless on
	// the port set value callback
	std::unordered_map<std::string, Port*> outputs;

	// pre_update is mostly used for input
	void pre_update(double dt);
	void update(double dt);

	void init(Part* in_part, Universe* in_universe);

	// Make sure AssetManager's correct current package is set,
	// otherwise script loading MAY fail!
	Machine(std::shared_ptr<cpptoml::table> init_toml, std::string pkg);
	~Machine();

	// Will only invalidate the wires that need to be invalidated
	// so adding new ports will not require re-wiring
	void define_ports();

	void add_input_port(const std::string& name, const std::string& type, sol::safe_function callback);
	void add_output_port(const std::string& name, const std::string& type);

	Port* get_input_port(const std::string& name);
	Port* get_output_port(const std::string& name);

	// Returns true once all input ports have received a value
	// (Disconnected ports act like if they have no value!)
	bool all_inputs_ready();

	PortResult write_to_port(const std::string& name, PortValue val);

};

