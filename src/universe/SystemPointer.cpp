#include "SystemPointer.h"

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

	if (elem->is_barycenter)
	{
		// TODO: Get the rotation to make the line joining the main and secondary
		// bodies the forward (X) axis
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
