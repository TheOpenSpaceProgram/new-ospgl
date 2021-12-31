#pragma once
#include <string>
#include <vector>
#include "StoredFluids.h"

class Machine;
class MachinePlumbing;
class GUISkin;

class MachinePlumbing;
class Pipe;

struct FluidPort
{
	// Useful pointer to have around to avoid endless lookups
	MachinePlumbing* in_machine;

	std::string id;
	// Numeric id is only used for junctions
	size_t numer_id;
	std::string gui_name;
	// Fluid ports must have a physical location
	std::string marker;
	// Position relative to the part
	glm::vec2 pos;
	// If true, the port is used as a flow port instead of true port (read fluid simulation notes)
	bool is_flow_port;
};


// Implements generic code for plumbing. May or may not be contained in a real machine.
class MachinePlumbing
{
private:

	bool can_add_ports;
	glm::ivec2 base_size;
	bool has_lua;

public:

	glm::ivec2 editor_position;
	int editor_rotation;

	// If not null, this is a real machine. if null, we are a plumbing only machine, which
	// are plumbing only machines which are serialized separately
	Machine* in_machine;
	// If not a machine this will be present and non-zero
	size_t plumbing_machine_id;


	glm::ivec2 get_base_size() { return base_size; }
	// Does not include rotation!
	void draw_diagram(void* vg, GUISkin* skin);

	std::vector<FluidPort> fluid_ports;

	// Used internally as all functions reside inside plumbing
	// It returns the table even if there's no table, fails silently optionally
	sol::table get_lua_plumbing(bool silent_fail = false);

	bool has_ports() const { return !fluid_ports.empty(); }
	bool has_lua_plumbing() const { return has_lua; }

	bool is_requester();
	void fluid_update();

	// Only makes sense on true ports
	float get_pressure(const std::string& port);
	// Get the pressure DROP. You can return negative values if you are a pump, etc...
	float get_pressure_drop(const std::string& from, const std::string& to, float cur_P);

	// Volumes are in m^3, if you cannot supply enough, don't!
	StoredFluids out_flow(std::string port, float volume, bool do_flow);
	// Return what you couldn't accept, ideally:
	// - Accept gases as they are compressible
	// - Feel free to refuse liquids if no more can fit
	StoredFluids in_flow(std::string port, const StoredFluids& in, bool do_flow);

	explicit MachinePlumbing(Machine* sm) : in_machine(sm) {}
	void init(const cpptoml::table& toml);
	void create_port(std::string id, std::string marker, std::string ui_name, float x, float y, bool is_flow_port);
	// Return ports which are "physically" connected to the given one. Only called on flow ports.
	std::vector<FluidPort*> get_connected_ports(const std::string& port);
	FluidPort* get_port_by_id(const std::string& name);
	// Corrects a position in local coordinates to global in the editor. Used for ports
	glm::ivec2 correct_editor_pos(glm::ivec2 pos);
	glm::ivec2 get_size(bool expand = false, bool rotate = true);
};

