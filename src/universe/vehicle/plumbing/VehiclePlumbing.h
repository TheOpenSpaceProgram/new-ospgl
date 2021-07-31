#pragma once
#include "../part/Machine.h"
#include <unordered_map>

class Vehicle;

struct PipeJunction;

// A pipe joins either a machine to a machine or
// a junction to a machine (Junction-junction connections
// are simplified to a single junction).
// (Fluid tanks are machines)
struct Pipe
{
	// For serialization and re-generation of the tree on changes
	size_t id;

	// mb may be null, then junction must be present
	// The pipe goes from ma to junction, although during building ma may also not be present
	// if the start of the pipe has been disconnected
	Machine *ma, *mb;
	PipeJunction* junction;
	std::string port_a, port_b;
	float surface;

	// Real-time updated
	float flow;

	// Editor only but serialized
	// They are helpful if your pipes get cluttered up and can be added as the
	// pipe is built
	std::vector<glm::vec2> waypoints;
	// Editor only and not serialized, jumps are visual only and
	// happen on "horizontal" segments of pipe which intersect a "vertical"
	// section
	std::vector<glm::vec2> jumps;
};

struct PipeJunction
{
	// For serialization and re-generation of the tree on changes, not used in runtime
	size_t id;

	// Editor only but serialized
	glm::ivec2 pos;
	// Editor only, same as parts
	int rotation;

	// The order matters for display purposes
	std::vector<Pipe*> pipes;

	size_t get_port_number() const {return pipes.size(); }
	glm::ivec2 get_size(bool extend = false, bool rotate = true) const;
};

// Do not keep for long! They store pointers to PipeJunction which
// may go invalid pretty quick, use in a single frame
struct PlumbingElement
{
	enum Type
	{
		EMPTY,
		MACHINE,
		JUNCTION
	};

	Type type;
	union {
		Machine* as_machine;
		PipeJunction* as_junction;
	};

	explicit PlumbingElement(Machine* machine);
	explicit PlumbingElement(PipeJunction* junction);
	PlumbingElement();

	// TODO: These only work with the plumbing element on the lhs!
	bool operator==(const Machine* m) const;
	bool operator==(const PipeJunction* j) const;
	bool operator==(const PlumbingElement& j) const;

	// The second value in the pair is the position of the port in the grid, including rotation and transforms
	std::vector<std::pair<FluidPort, glm::vec2>> get_ports();
	glm::ivec2 get_size(bool expand = false, bool rotate = true);
	glm::ivec2 get_pos();
	void set_pos(glm::ivec2 pos);
	int get_rotation();
	void set_rotation(int value);
};

// Vehicle plumbing allows connection of the fluid ports
// of machines. Unlike wires, plumbing connections have
// physical connection points and physical properties
// A vehicle's plumbing is made of a set of fluid tanks,
// connected via pipes into other fluid tanks or machines
// We also simulate junctions
// Unlike machines, pipes are lightweight so we hold them in
// contiguous memory. Modifying pipes or junctions requires
// regeneration of the tree SO DON'T DO IT MANUALLY!
// Junctions are not machines as they are not a physical part
class VehiclePlumbing
{
private:

	Vehicle* veh;

public:

	// These functions check and area of the plumbing grid for machines
	std::vector<PlumbingElement> grid_aabb_check(glm::vec2 start, glm::vec2 end, bool expand = false)
		{ return grid_aabb_check(start, end, {}, expand); }
	std::vector<PlumbingElement> grid_aabb_check(glm::vec2 start, glm::vec2 end, const std::vector<PlumbingElement>& ignore,
										  bool expand = false);

	std::vector<PlumbingElement> get_all_elements();

	std::vector<Pipe> pipes;
	// Junction id to its pipes, generated on load / modify to speed up the algorithm
	std::vector<PipeJunction> junctions;

	void update_pipes(float dt, Vehicle* in_vehicle);
	void junction_flow_rate(const PipeJunction& jnc, float dt);

	// Called when adding new parts, or merging vehicles (etc...)
	glm::ivec2 find_free_space(glm::ivec2 size);
	// Returns (0, 0) if none of the machines have plumbing
	glm::ivec2 get_plumbing_size_of(Part* p);

	// Returns an AABB (pos, size) with the bounds of used plumbing area by machines
	glm::ivec4 get_plumbing_bounds();

	explicit VehiclePlumbing(Vehicle* in_vehicle);

};