#pragma once
#include <cpptoml.h>
#include <universe/Universe.h>
#include <util/LuaUtil.h>
#include <lua/LuaCore.h>
#include "../plumbing/PlumbingMachine.h"

class Vehicle;
class Part;
class Piece;

// Machines implement functionality for parts in lua
// They have the global "osp", and "machine" (themselves)
// you may access the part from machine.in_part, and other stuff from osp.X
class Machine
{
friend class Vehicle;

private:


	std::vector<Machine*> get_connected_if(std::function<bool(Machine*)> fnc, bool include_this);

	std::string in_pkg, name;
	AssetHandle<Image> default_icon;

	// We precache this on Vehicle::remove_outdated
	bool piece_missing;

	bool paused;
	bool step;

public:

	// An unique id assigned at runtime to identify stuff like ImGui windows, etc...
	// Not very elegant solution but useful for the debug stuff
	int64_t runtime_uid;
	// String to be used in part->get_machine() to obtain this machine
	std::string in_part_id;

	// Always present, but may not be used
	PlumbingMachine plumbing;

	sol::environment env;
	sol::state* lua_state;

	Part* in_part;
	std::string assigned_piece;

	std::shared_ptr<cpptoml::table> init_toml;

	std::unordered_map<std::string, sol::table> interfaces;
	// May be "", in that case the machine is "centered" on the piece
	std::string editor_location_marker;
	bool editor_hidden;

	// A machine may not be enabled if its part goes missing!
	// We update anyway, this is informative to the piece lua, but
	// keep in mind a missing machine will lose all wire and pipe connetions!
	bool is_enabled();

	// pre_update is mostly used for input
	void pre_update(double dt);
	void update(double dt);
	void editor_update(double dt);

	void physics_update(double dt);

	void init(sol::state* lua_state, Part* in_part);

	void load_interface(const std::string& name, sol::table n_table);

	std::vector<Machine*> get_all_wired_machines(bool include_this = true);
	std::vector<Machine*> get_wired_machines_with(const std::vector<std::string>& interfaces, bool include_this = true);
	std::vector<sol::table> get_wired_interfaces(const std::string& type, bool include_this = true);

	sol::table get_interface(const std::string& name);

	AssetHandle<Image> get_icon();

	std::string get_pkg();
	std::string get_name();
	std::string get_id();
	// Don't create your own window in lua! We create a standard named one
	// Returns wether we are still open
	bool draw_imgui();

	// Make sure AssetManager's correct current package is set,
	// otherwise script loading MAY fail!
	Machine(std::shared_ptr<cpptoml::table> init_toml, std::string pkg);
	~Machine();

};


