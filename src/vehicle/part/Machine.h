#pragma once
#include <sol.hpp>
#include <cpptoml.h>

class Vehicle;
class Part;

// TODO: Define this later
class World
{

};

// Machines implement functionality for parts in lua
// Any non-implemented function will simply be ignored!
//
//	- function load(data: toml)			-> void
//	- function unload()					-> toml
//	- function update(dt: number)		-> void
//
// You always have access to these global objects:
//	- part: The part this machine is inside of (other pieces, physics, wiring, config...)
//	- universe: The world this machine is inside of (solar system, other vehicles...)
//	- machine: This machine, and its config
//	- To access vehicle, use machine.vehicle, as it may change
//
// Lua standard libraries loaded are '
// Only the 'core' library is loaded by default, load other libraries by load_lib("wathever")
// at the start of the script
//
// Initialization:
//	You should do basic initialization outside of all functions, in the script, 
//	using the config stored in 'machine'
//	
//	'load' gets called when the vehicle is actually loaded, and its data deserialized
//	You should load any data written by 'unload' there
//
class Machine
{
private:


	struct ErrorHandling
	{
		bool is_init_fatal;
		bool is_load_fatal;
		bool is_update_fatal;

		ErrorHandling()
		{
			is_init_fatal = true;
			is_load_fatal = true;
			is_update_fatal = false;
		}
	};

	sol::state lua_state;
	std::string file_name;

	void handle_lua_error(const sol::protected_function_result& pfr, const std::string& msg, bool fatal);

public:

	Vehicle* vehicle;

	std::shared_ptr<cpptoml::table> init_toml;

	ErrorHandling error_handling;

	void update(double dt);
	std::shared_ptr<cpptoml::table> serialize();
	void deserialize(const cpptoml::table& toml);

	// Make sure AssetManager's correct current package is set,
	// otherwise script loading MAY fail!
	Machine(std::shared_ptr<cpptoml::table> init_toml, Vehicle* in_vehicle, Part* in_part, World* in_world);
	~Machine();
};

