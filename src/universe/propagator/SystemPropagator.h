#pragma once
#include "../UniverseDefinitions.h"
#include "Propagable.h"


// Propagates N-body systems and can also handle vessels and non-attracting bodies
class SystemPropagator
{
protected:
	StateVector* st_vector;
	LightStateVector* lst_vector;
	// If nullptr, it's nbody. Same number of elements as lst_vector
	TrajectoryVector* trj_vector;

public:


	void bind_to(Propagable* system)
	{
		this->st_vector = system->get_massful_states();
		this->lst_vector = system->get_light_states();
		this->trj_vector = system->get_trajectories();
		init();
	}

	void bind_to(StateVector* s, LightStateVector* l, TrajectoryVector* t)
	{
		this->st_vector = s;
		this->lst_vector = l;
		this->trj_vector = t;
		init();
	}


	virtual void init() {}
	// Propagates the system, including non-nbody bodies
	virtual void propagate(double dt) = 0;

	virtual ~SystemPropagator() = default;
};
