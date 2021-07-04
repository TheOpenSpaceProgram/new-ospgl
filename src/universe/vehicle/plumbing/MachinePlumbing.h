#pragma once
#include <string>
#include <vector>
#include "StoredFluids.h"

class Machine;

struct FluidPort
{
	std::string id;
	// Fluid ports must have a physical location
	std::string marker;
	// Position in the plumbing editor, set only during init or if the
	// ports change during editor operations
	glm::vec2 pos;
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

public:

	glm::ivec2 editor_position;
	glm::ivec2 get_editor_size();
	void draw_diagram(void* vg);

	std::vector<FluidPort> fluid_ports;

	// Used internally as all functions reside inside plumbing
	// It returns the table even if there's no table, fails silently optionally
	sol::table get_lua_plumbing(bool silent_fail = false);

	bool has_ports() const { return !fluid_ports.empty(); }
	// TODO: we could cache the result as it won't disappear mid-game
	bool has_lua_plumbing();

	bool is_requester();
	void fluid_update();

	float get_pressure(std::string port);

	// Volumes are in m^3, if you cannot supply enough, don't!
	StoredFluids out_flow(std::string port, float volume);
	// Return what you couldn't accept, ideally:
	// - Accept gases as they are compressible
	// - Feel free to refuse liquids if no more can fit
	StoredFluids in_flow(std::string port, StoredFluids in);

	// Free volume for liquids (you should accept infinite gases, or explode...)
	// Make sure in_flow accepts exactly this, otherwise fluids will break
	float get_free_volume();

	explicit MachinePlumbing(Machine* sm) : machine(sm) {}
	void init(const cpptoml::table& toml);

	void create_port(std::string id, std::string marker, float x, float y);


};

