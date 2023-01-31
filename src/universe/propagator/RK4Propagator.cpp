#include "RK4Propagator.h"

void RK4Propagator::f(SolVec *target, const SolVec& eval_p)
{
	// target is (x', v')
	// eval_p is (x, v)
	// so that x' = v, v' = accelerations(x)

	// Evaluates accelerations(x) into v'
	// TODO: Easy optimization, gravity is symmetric!
	for(size_t i = 0; i < size; i++)
	{
		(*target)[i].second = glm::dvec3(0, 0, 0);
		for(size_t j = 0; j < size; j++)
		{
			if(i == j)
				continue;

			double mass_j = masses[j];
			glm::dvec3 diff = eval_p[j].first - eval_p[i].first;
			double dist2 = glm::length2(diff);

			glm::dvec3 diffn = diff / glm::sqrt(dist2);
			double am = (G * mass_j) / dist2;
			(*target)[i].second += diffn * am;
		}

		// Evaluates v into x' (integrate velocity)
		(*target)[i].first = eval_p[i].second;
	}
}

void RK4Propagator::fl(SolVec* target, const SolVec& light_p, const SolVec& eval_p)
{
	// target is (x', v')
	// light_p is (x, v) of non-attracting bodies
	// eval_p is (x, v) of attracting bodies
	for(size_t i = 0; i < lsize; i++)
	{
		(*target)[i].second = glm::dvec3(0, 0, 0);
		for(size_t j = 0; j < size; j++)
		{
			double mass_j = masses[j];
			glm::dvec3 diff = eval_p[j].first - light_p[i].first;
			double dist2 = glm::length2(diff);

			glm::dvec3 diffn = diff / glm::sqrt(dist2);
			double am = (G * mass_j) / dist2;
			(*target)[i].second += diffn * am;
		}

		// Evaluates v into x' (integrate velocity)
		(*target)[i].first = light_p[i].second;
	}
}

// TODO: This is a "textbook" readable implementation, can be optimzied greatly
// by merging many loops if perfomance is an issue
void RK4Propagator::propagate(double dt)
{
	resize();
	double hdt = dt * 0.5;

	// First run RK4 on the massive states, storing intermediate positions
	// as these will be used by the vessels
	// Copy start positions and masses
	for(size_t i = 0; i < size; i++)
	{
		u0[i].first = (*st_vector)[i].pos;
		u0[i].second = (*st_vector)[i].vel;
		masses[i] = (*st_vector)[i].mass;
	}
	f(&C1, u0);
	set_buffer(&buffer1, C1, u0, hdt, size);
	f(&C2, buffer1);
	set_buffer(&buffer2, C2, u0, hdt, size);
	f(&C3, buffer2);
	set_buffer(&buffer3, C3, u0, dt, size);
	f(&C4, buffer3);

	// We can now step the system
	for(size_t i = 0; i < size; i++)
	{
		double h = dt * (1.0 / 6.0);
		(*st_vector)[i].pos = u0[i].first + h * (C1[i].first + 2.0 * C2[i].first + 2.0 * C3[i].first + C4[i].first);
		(*st_vector)[i].vel = u0[i].second + h * (C1[i].second + 2.0 * C2[i].second + 2.0 * C3[i].second + C4[i].second);
	}

	// Then run RK4 on the light states, using intermediate states
	for(size_t i = 0; i < lsize; i++)
	{
		lu0[i].first = (*lst_vector)[i].pos;
		lu0[i].second = (*lst_vector)[i].vel;
	}
	fl(&lC1, lu0, u0);
	set_buffer(&lbuffer1, lC1, lu0, hdt, lsize);
	fl(&lC2, lbuffer1, buffer1);
	set_buffer(&lbuffer2, lC2, lu0, hdt, lsize);
	fl(&lC3, lbuffer2, buffer2);
	set_buffer(&lbuffer3, lC3, lu0, dt, lsize);
	fl(&lC4, lbuffer3, buffer3);

	for(size_t i = 0; i < lsize; i++)
	{
		double h = dt * (1.0 / 6.0);
		(*lst_vector)[i].pos = lu0[i].first + h * (lC1[i].first + 2.0 * lC2[i].first + 2.0 * lC3[i].first + lC4[i].first);
		(*lst_vector)[i].vel = lu0[i].second + h * (lC1[i].second + 2.0 * lC2[i].second + 2.0 * lC3[i].second + lC4[i].second);
	}
}

void RK4Propagator::resize()
{
	if(C1.size() != st_vector->size())
	{
		size = st_vector->size();
		C1.resize(size);
		C2.resize(size);
		C3.resize(size);
		C4.resize(size);
		u0.resize(size);
		buffer1.resize(size);
		buffer2.resize(size);
		buffer3.resize(size);
		masses.resize(size);
	}

	if(lC1.size() != lst_vector->size())
	{
		lsize = lst_vector->size();
		lC1.resize(lsize);
		lC2.resize(lsize);
		lC3.resize(lsize);
		lC4.resize(lsize);
		lu0.resize(lsize);
		lbuffer1.resize(lsize);
		lbuffer2.resize(lsize);
		lbuffer3.resize(lsize);
	}

}

void RK4Propagator::set_buffer(RK4Propagator::SolVec* target, const RK4Propagator::SolVec &C, const RK4Propagator::SolVec& eu0,
							   double dt, size_t s)
{
	// buffer = u0 + dt * C
	for(size_t i = 0; i < s; i++)
	{
		(*target)[i].first = eu0[i].first + C[i].first * dt;
		(*target)[i].second = eu0[i].second + C[i].second * dt;
	}
}
