#pragma once
#include "PlanetarySystem.h"

struct SystemPointer
{
private:


	PlanetarySystem* system;

public:

	bool is_vessel;
	int id;

	Vessel* get_vessel() const;
	SystemElement* get_element() const;

	glm::dvec3 get_position_now() const;
	glm::dvec3 get_forward_now() const;
	// On planets this is the north pole
	glm::dvec3 get_up_now() const;

	glm::dvec3 get_velocity_now() const;

	SystemPointer(PlanetarySystem* sys)
	{
		this->system = sys;
	}
};