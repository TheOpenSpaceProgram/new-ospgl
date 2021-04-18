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
public:

	std::vector<Pipe> pipes;
	// Junction id to its pipes, generated on load / modify to speed up the algorithm
	std::vector<PipeJunction> junctions;

	void update_pipes(float dt, Vehicle* in_vehicle);
	void junction_flow_rate(const PipeJunction& jnc, float dt);

};