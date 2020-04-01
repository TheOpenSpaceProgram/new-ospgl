#pragma once
#include <sol.hpp>
#include <cpptoml.h>
#include "../../universe/Universe.h"
#include "../../util/LuaUtil.h"
#include "../../lua/LuaCore.h"

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



	sol::state lua_state;
	Part* in_part;
	bool is_defining_ports;

	struct PortDefinition
	{
		std::string name;
		std::string type;
		bool output;

	};

	std::vector<PortDefinition> new_ports;

public:


	std::shared_ptr<cpptoml::table> init_toml;

	std::vector<Port*> ports;

	void update(double dt);
	//void wire_receive(std::string wire_id, );

	void init(Part* in_part, Universe* in_universe);

	// Make sure AssetManager's correct current package is set,
	// otherwise script loading MAY fail!
	Machine(std::shared_ptr<cpptoml::table> init_toml, std::string pkg);
	~Machine();

	// Will only invalidate the wires that need to be invalidated
	// so adding new ports will not require re-wiring
	void define_ports();

	void add_port(const std::string& name, const std::string& type, bool output);

	Port* get_input_port(const std::string& name);
	Port* get_output_port(const std::string& name);
};

