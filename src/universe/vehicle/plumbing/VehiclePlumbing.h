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
	Machine *ma, *mb;
	PipeJunction* junction;
	std::string port_a, port_b;
	float surface;

	// Real-time updated
	float flow;

	// Editor only but serialized
	std::vector<glm::vec2> waypoints;
	// Editor only and not serialized, jumps are visual only and
	// happen on horizontal segments of pipe which intersect a vertical
	// section
	std::vector<glm::vec2> jumps;
};

struct PipeJunction
{
	// NOT SERIALIZED, generated on load to speed up the algorithm
	std::vector<Pipe*> pipes;
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
class VehiclePlumbing
{
private:

	Vehicle* veh;

public:

	// These functions check and area of the plumbing grid for machines
	std::vector<Machine*> grid_aabb_check(glm::vec2 start, glm::vec2 end, bool expand = false) const
		{ return grid_aabb_check(start, end, {}, expand); }
	std::vector<Machine*> grid_aabb_check(glm::vec2 start, glm::vec2 end, const std::vector<Machine*>& ignore,
										  bool expand = false) const;

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