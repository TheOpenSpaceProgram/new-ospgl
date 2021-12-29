#pragma once
#include <string>
#include <vector>
#include "StoredFluids.h"

class Machine;
class MachinePlumbing;
class GUISkin;

class MachinePlumbing;

struct FluidPort
{
	// Useful pointer to have around
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


// Machine fluid ports are determined by the lua file (in init!), which may offer
// customization options through the toml (for example, number of inlets in
// an engine). Fluid ports may not change during flight, but they may change
// in the editor, as this will break the fluid connections as a port may disappear
// or change name.
// A machine is considered to not have plumbing if it has no fluid ports
class MachinePlumbing
{
private:

	Machine* machine;
	bool can_add_ports;
	glm::ivec2 base_size;
	bool has_lua;

public:

	glm::ivec2 editor_position;
	// 0 means no rotation, 1 90º, 2 180º, 3 270º
	int editor_rotation = 0;
	// Expand extends the size by 1 in both directions so parts must be spaced out
	glm::ivec2 get_editor_size(bool expand = false, bool rotate = true) const;
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

	float get_pressure(const std::string& port);

	// Volumes are in m^3, if you cannot supply enough, don't!
	StoredFluids out_flow(std::string port, float volume, bool do_flow);
	// Return what you couldn't accept, ideally:
	// - Accept gases as they are compressible
	// - Feel free to refuse liquids if no more can fit
	StoredFluids in_flow(std::string port, const StoredFluids& in, bool do_flow);

	explicit MachinePlumbing(Machine* sm) : machine(sm) {}
	void init(const cpptoml::table& toml);
	void create_port(std::string id, std::string marker, std::string ui_name, float x, float y, bool is_flow_port);
	// Return ports which are "physically" connected to the given one. Only called on flow ports.
	std::vector<FluidPort*> get_connected_ports(std::string port);

	glm::vec2 get_port_position(std::string id);

};

