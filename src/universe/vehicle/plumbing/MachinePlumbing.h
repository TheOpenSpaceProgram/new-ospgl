#pragma once
#include <string>
#include <vector>
#include "StoredFluids.h"


struct FluidPort
{
	std::string id;
	// Fluid ports must have a physical location
	std::string marker;
};

// Each machine may have any number of fluid ports, defined in the
// machine toml file with an ID and a marker ID for location. The count and
// IDs of the plumbing connections ARE NOT determined by the machine, which
// can impose limiations on the ammount of connections and ID names. Machines
// should raise errors on any incosistency
class MachinePlumbing
{
public:


	std::vector<FluidPort> fluid_ports;

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


};

