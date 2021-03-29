#pragma once
#include "../part/Machine.h"
#include <unordered_map>

class Vehicle;

// A pipe joins either a machine to a machine, or
// a junction to a machine (Junction-junction connections
// are simplified to a single junction)
struct Pipe
{
	size_t id;
	// mb may be null, then junction must be present
	Machine *ma, *mb;
	size_t junction;
	std::string port_a, port_b;
	float surface;

	// Real-time updated
	float flow;
};

struct PipeJunction
{
	// NOT SERIALIZED, generated on load to speed up the algorithm
	std::vector<size_t> pipes;
};


// Vehicle plumbing allows connection of the fluid ports
// of machines. Unlike wires, plumbing connections have
// physical connection points and physical properties
// A vehicle's plumbing is made of a set of fluid tanks,
// connected via pipes into other fluid tanks or machines
// We also simulate junctions
class VehiclePlumbing
{
public:

	std::unordered_map<size_t, Pipe> pipes;
	// Junction id to its pipes, generated on load / modify to speed up the algorithm
	std::unordered_map<size_t, PipeJunction> junctions;

	// m^3/s Positive flow rate means from a to b, negative
	// from b to a. We assume that more dense fluids
	// dominate the flow, homogeinizing it
	static float get_flow_rate(Pipe* p);

	void update_pipes(Vehicle* in_vehicle);
	void junction_flow_rate(PipeJunction& jnc);

};