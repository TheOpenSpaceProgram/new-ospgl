#pragma once

#include "vessel/Vessel.h"
#include "element/SystemElement.h"

class PlanetarySystem;

struct SystemPointer
{
private:



public:

	PlanetarySystem* system;

	bool is_vessel;
	int id;

	Vessel* get_vessel() const;
	SystemElement* get_element() const;

	glm::dvec3 get_position_now() const;
	glm::dvec3 get_forward_now() const;
	// On planets this is the north pole
	glm::dvec3 get_up_now() const;

	glm::dvec3 get_velocity_now() const;

	double get_mass() const;

	SystemPointer(PlanetarySystem* sys);

	// From body name
	SystemPointer(PlanetarySystem* sys, std::string body_name);


	SystemPointer(PlanetarySystem* sys, int id, bool is_vessel = true);

};