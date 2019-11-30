#pragma once
#include "PlanetarySystem.h"

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

	SystemPointer(PlanetarySystem* sys)
	{
		this->system = sys;
	}

	// From body name
	SystemPointer(PlanetarySystem* sys, std::string body_name)
	{
		this->system = sys;
	}

	SystemPointer(PlanetarySystem* sys, int id, bool is_vessel = true)
	{
		this->system = sys;
		this->id = id;
		this->is_vessel = is_vessel;
	}
};