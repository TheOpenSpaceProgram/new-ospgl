#include "RK4Interpolated.h"
#include "../PlanetarySystem.h"
#include "../vessel/Vessel.h"


RK4Interpolated::Derivative RK4Interpolated::sample(CartesianState s0, Derivative d, double dt, PosVector& vec)
{
	CartesianState s;
	s.pos = s0.pos + d.dx * dt;
	s.vel = s0.vel + d.dv * dt;

	Derivative o;
	o.dx = s.vel;
	o.dv = acceleration(s.pos, vec);

	return o;
}

glm::dvec3 RK4Interpolated::acceleration(glm::dvec3 p, PosVector& vec)
{
	glm::dvec3 acc = glm::dvec3(0.0, 0.0, 0.0);

	for (size_t i = 0; i < vec.size(); i++)
	{
		if (masses[i] != 0.0)
		{
			glm::dvec3 diff = vec[i] - p;
			glm::dvec3 diffn = glm::normalize(diff);
			double dist2 = glm::length2(diff);

			double am = (G * masses[i]) / dist2;

			acc += am * diffn;
		}
	}

	return acc;
}

void RK4Interpolated::initialize(PlanetarySystem* system, size_t body_count)
{
	this->sys = system;
	t0_pos.resize(body_count + 1);
	t1_pos.resize(body_count + 1);
	t05_pos.resize(body_count + 1);

	masses.resize(body_count + 1);

	masses[0] = system->star_mass;

	// Obtain masses
	for (size_t i = 0; i < body_count; i++)
	{
		if (system->elements[i].is_barycenter)
		{
			masses[i + 1] = 0.0;
		}
		else
		{
			masses[i + 1] = system->elements[i].as_body->config.mass;
		}
	}
}

void RK4Interpolated::prepare(double t0, double tstep)
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
}

void RK4Interpolated::propagate(Vessel* v)
{
	Derivative zero = Derivative();
	zero.dx = glm::dvec3(0.0); zero.dv = glm::dvec3(0.0);

	CartesianState s0 = v->state;
	Derivative k1, k2, k3, k4;

	k1 = sample(s0, zero, tstep * 0.0, t0_pos);
	k2 = sample(s0, k1, tstep * 0.5, t05_pos);
	k3 = sample(s0, k2, tstep * 0.5, t05_pos);
	k4 = sample(s0, k3, tstep * 1.0, t1_pos);

	glm::dvec3 dxdt = (1.0 / 6.0) * (k1.dx + 2.0 * k2.dx + 2.0 * k3.dx + k4.dx);
	glm::dvec3 dvdt = (1.0 / 6.0) * (k1.dv + 2.0 * k2.dv + 2.0 * k3.dv + k4.dv);

	v->state.pos += dxdt * tstep;
	v->state.vel += dvdt * tstep;
}
