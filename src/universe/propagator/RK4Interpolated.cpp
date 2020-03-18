#include "RK4Interpolated.h"
#include "../PlanetarySystem.h"
#include "../vessel/Vessel.h"

template<bool get_closest>
RK4Interpolated::Derivative RK4Interpolated::sample(CartesianState s0, Derivative d, double dt, PosVector& vec, size_t* closest)
{
	CartesianState s;
	s.pos = s0.pos + d.dx * dt;
	s.vel = s0.vel + d.dv * dt;

	Derivative o;
	o.dx = s.vel;
	o.dv = acceleration<get_closest>(s.pos, vec, closest);

	return o;
}

template<bool get_closest>
glm::dvec3 RK4Interpolated::acceleration(glm::dvec3 p, PosVector& vec, size_t* closest)
{
	glm::dvec3 acc = glm::dvec3(0.0, 0.0, 0.0);

	double min_distance2 = std::numeric_limits<double>::infinity();

	for (size_t i = 0; i < vec.size(); i++)
	{
		if (masses[i] != 0.0)
		{
			glm::dvec3 diff = vec[i] - p;
			glm::dvec3 diffn = glm::normalize(diff);
			double dist2 = glm::length2(diff);

			double am = (G * masses[i]) / dist2;

			acc += am * diffn;

			// Tiny tiny optimization, but worth it
			if constexpr (get_closest)
			{
				if (dist2 < min_distance2)
				{
					min_distance2 = dist2;
					*closest = i;
				}
			}

		}
	}

	return acc;
}

void RK4Interpolated::initialize(PlanetarySystem* system, size_t body_count)
{
	this->sys = system;
	t0_pos.resize(body_count);
	t1_pos.resize(body_count);
	t05_pos.resize(body_count);

	masses.resize(body_count);

	// Obtain masses
	for (size_t i = 0; i < body_count; i++)
	{
		if (system->elements[i].type == SystemElement::BARYCENTER)
		{
			masses[i] = 0.0;
		}
		else
		{
			masses[i] = system->elements[i].get_mass();
		}
	}
}

void RK4Interpolated::prepare(double t0, double tstep, PosVector& out_pos)
{
	this->t0 = t0;
	this->t1 = t0 + tstep;
	this->tstep = tstep;

	sys->compute_positions(t0, t0_pos, 1e-6);
	sys->compute_positions(t1, t1_pos, 1e-6);

	for (size_t i = 0; i < t0_pos.size(); i++)
	{
		glm::dvec3 a = t0_pos[i];
		glm::dvec3 b = t1_pos[i];
		t05_pos[i] = glm::mix(a, b, 0.5);

	}

	out_pos = this->t0_pos;
}

size_t RK4Interpolated::propagate(CartesianState* state)
{
	Derivative zero = Derivative();
	zero.dx = glm::dvec3(0.0); zero.dv = glm::dvec3(0.0);

	CartesianState s0 = *state;
	Derivative k1, k2, k3, k4;
	
	size_t closest;

	k1 = sample<true>(s0, zero, tstep * 0.0, t0_pos, &closest);
	k2 = sample<false>(s0, k1, tstep * 0.5, t05_pos, nullptr);
	k3 = sample<false>(s0, k2, tstep * 0.5, t05_pos, nullptr);
	k4 = sample<false>(s0, k3, tstep * 1.0, t1_pos, nullptr);

	glm::dvec3 dxdt = (1.0 / 6.0) * (k1.dx + 2.0 * k2.dx + 2.0 * k3.dx + k4.dx);
	glm::dvec3 dvdt = (1.0 / 6.0) * (k1.dv + 2.0 * k2.dv + 2.0 * k3.dv + k4.dv);

	state->pos += dxdt * tstep;
	state->vel += dvdt * tstep;

	return closest;
}
