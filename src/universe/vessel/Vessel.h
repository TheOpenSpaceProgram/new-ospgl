#pragma once
#include "../kepler/KeplerElements.h"
#include "../SystemElement.h"
#include "../UniverseDefinitions.h"
class PlanetarySystem;

// Everything is always relative to the global coordinate frame
// Forward is 'Y' axis (as rockets are usually vertical)
// TODO: Maybe use 'X' as forward? More consistent
// Units are as usual, but angular_momentum is expressed in (rad / s)!
class Vessel
{

public:
	
	CartesianState state;
	glm::dquat rotation;
	glm::dvec3 angular_momentum; //< Length = Angular velocity (rad / s)

	// Called after every state propagation
	void simulate(ElementVector elements, PosVector positions, double star_radius, size_t closest, double dt);

	glm::dvec3 get_forward();
	glm::dvec3 get_right();
	glm::dvec3 get_up();

	void apply_torque(glm::dvec3 torque);

	void draw_debug();

	Vessel();
	~Vessel();

	// ----------------------------------------
	// On Rails stuff (TODO)
	// ----------------------------------------
	bool on_rails;
	KeplerElements on_rails_orbit;
	int parent_index;

};

