#include "SystemPointer.h"
#include "PlanetarySystem.h"

Vessel* SystemPointer::get_vessel() const
{
	if (is_vessel)
	{
		return &system->vessels[id];
	}
	else
	{
		logger->warn("Tried to unreference a SystemPointer as vessel, it was not a vessel");
		return nullptr;
	}
}

SystemElement* SystemPointer::get_element() const
{
	if (!is_vessel)
	{
		return &system->elements[id];
	}
	else
	{
		logger->warn("Tried to unreference a SystemPointer as element, it was not a element");
		return nullptr;
	}
}

glm::dvec3 SystemPointer::get_position_now() const
{
	if (is_vessel)
	{
		return system->vessels[id].state.pos;
	}
	else
	{
		return system->states_now[id].pos;
	}
}

static glm::dvec3 transform_vector(glm::dvec3 vec, const SystemElement* elem, double t)
{

	if (elem->type == SystemElement::BARYCENTER)
	{
		// TODO: Get the rotation to make the line joining the main and secondary
		// bodies the forward (X) axis
		return vec;
	}
	else if (elem->type == SystemElement::STAR)
	{
		// Star is always fixed
		return vec;
	}
	else
	{
		return elem->as_body->build_rotation_matrix(t) * glm::dvec4(vec, 1.0);
	}
}

glm::dvec3 SystemPointer::get_forward_now() const
{
	if (is_vessel)
	{
		return system->vessels[id].get_forward();
	}
	else
	{
		const SystemElement* elem = get_element();
		return transform_vector(glm::dvec3(1.0, 0.0, 0.0), elem, system->t);
	}
}

glm::dvec3 SystemPointer::get_up_now() const
{
	if (is_vessel)
	{
		return system->vessels[id].get_up();
	}
	else
	{
		const SystemElement* elem = get_element();
		return transform_vector(glm::dvec3(0.0, 1.0, 0.0), elem, system->t);
	}
}

glm::dvec3 SystemPointer::get_velocity_now() const
{
	if (is_vessel)
	{
		return get_vessel()->state.vel;
	}
	else
	{
		return system->states_now[id].vel;
	}
}

double SystemPointer::get_mass() const
{
	if (is_vessel)
	{
		return get_vessel()->mass;
	}
	else
	{
		return get_element()->get_mass(false, true);
	}
}

SystemPointer::SystemPointer(PlanetarySystem * sys)
{
	this->system = sys;
	// Solar focus
	this->id = 0;
	this->is_vessel = false;
}

SystemPointer::SystemPointer(PlanetarySystem * sys, std::string body_name)
{
	this->system = sys;
	this->id = (int)sys->name_to_index[body_name];
	this->is_vessel = false;
}

SystemPointer::SystemPointer(PlanetarySystem * sys, int id, bool is_vessel)
{
	this->system = sys;
	this->id = id;
	this->is_vessel = is_vessel;
}
