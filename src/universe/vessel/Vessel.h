#pragma once
#include "../kepler/KeplerElements.h"
#include "../SystemElement.h"

class PlanetarySystem;

// Everything is always relative to the global coordinate frame
// Units are as usual, but angular_momentum is expressed in (rad / s)!
class Vessel
{

public:
	
	using StateVector = std::vector<CartesianState>;
	using ElementVector = std::vector<SystemElement>;

	CartesianState state;
	glm::dquat rotation;
	glm::dvec3 angular_momentum; //< Length = Angular velocity (rad / s)

	Vessel();
	~Vessel();

	// ----------------------------------------
	// On Rails stuff (TODO)
	// ----------------------------------------
	bool on_rails;
	KeplerElements on_rails_orbit;
	int parent_index;

};

