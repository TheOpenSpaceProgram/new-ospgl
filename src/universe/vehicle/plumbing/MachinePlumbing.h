#pragma once
#include <string>
#include <vector>


struct FluidPort
{
	std::string id;
	// Fluid ports must have a physical location
	std::string marker;
};


class MachinePlumbing
{
public:


	std::vector<FluidPort> fluid_ports;

	bool is_requester();
	void fluid_update();

};

