#include "RK4Propagator.h"
#include <universe/PlanetarySystem.h>

template<bool RETURN_CLOSEST = false>
static size_t acceleration(int i, CartesianState prop, const std::vector<CartesianState>& states, size_t count, glm::dvec3& out_a)
{
	size_t closest = 0;
	double closest_dist = 10e100;

	for(size_t j = 0; j < count; j++)
	{
		if(i == (int)j) continue;

		glm::dvec3 dx = states[j].pos - prop.pos;
		double dist = glm::length(dx);
		// We use dist3 to normalize the vector, too
		double dist3 = dist * dist * dist;
		out_a += G * (states[j].mass / dist3) * dx;

		if constexpr(RETURN_CLOSEST)
		{
			if (dist < closest_dist)
			{
				closest = j;
				closest_dist = dist;
			}
		}
	}

	return closest;
}

size_t RK4Propagator::propagate(CartesianState* state, const std::vector<CartesianState>& states, double dt)
{
	return 0;
}

void RK4Propagator::propagate(std::vector<CartesianState>& states, double dt)
{
	// TODO: Implement RK4, this is Euler for a quick proof of concept
	for(size_t i = 0; i < states.size(); i++)
	{
		states[i].pos += states[i].vel * dt;
	}

	std::vector<CartesianState> t0_snap = states;
	for(size_t i = 0; i < states.size(); i++)
	{
		auto acc = glm::dvec3(0.0);
		acceleration(i, states[i], t0_snap, system->nbody_count, acc);
		states[i].vel += acc;
	}
}

void RK4Propagator::initialize(PlanetarySystem *s)
{
	system = s;
}

