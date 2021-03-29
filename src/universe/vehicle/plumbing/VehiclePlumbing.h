#pragma once
#include "../part/Machine.h"
#include <unordered_map>

class Vehicle;

struct Pipe
{
	Machine* ma, mb;
	std::string port_a, port_b;
	float radius;
};

struct PipeJunction
{
	Pipe *a, *b, *c;
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

	std::vector<Pipe> pipes;
	std::vector<PipeJunction> junctions;

	// m^3/s Positive flow rate means from a to b, negative
	// from b to a. We assume that more dense fluids
	// dominate the flow, homogeinizing it
	static float get_flow_rate(Pipe* p);

	void update_pipes(Vehicle* in_vehicle);
	std::vector<float> junction_flow_rate();

};