#include "PlanetarySystem.h"

glm::dvec3 compute_state(double t, double tol, double star_mass, PlanetaryBody* body, std::vector<glm::dvec3>* other_states)
{
	double parent_mass;
	glm::dvec3 offset = glm::dvec3(0.0, 0.0, 0.0);
	if (body->parent == nullptr)
	{
		parent_mass = star_mass;
	}
	else 
	{
		parent_mass = body->parent->config.mass;
		offset = (*other_states)[body->parent->index];
	}

	KeplerElements elems = body->orbit.to_elements_at(t, body->config.mass, parent_mass, tol);
	glm::dvec3 pos_rel = elems.get_position();

	return pos_rel + offset;
}


void PlanetarySystem::compute_states(double t, std::vector<glm::dvec3>& out, double tol)
{
	for (size_t i = 0; i < out.size(); i++)
	{
		out[i] = compute_state(t, tol, star_mass, &bodies[i], &out);
	}
}

void PlanetarySystem::compute_sois(double t)
{
	for (size_t i = 0; i < bodies.size(); i++)
	{
		double smajor_axis = bodies[i].orbit.to_orbit_at(t).smajor_axis;

		double parent_mass = star_mass;
		if (bodies[i].parent != nullptr)
		{
			parent_mass = bodies[i].parent->config.mass;
		}

		bodies[i].soi_radius = smajor_axis * pow(bodies[i].config.mass / parent_mass, 0.4);
	}
}

PlanetarySystem::PlanetarySystem()
{
}


PlanetarySystem::~PlanetarySystem()
{
}
