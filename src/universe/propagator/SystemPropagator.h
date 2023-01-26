#pragma once
#include "../UniverseDefinitions.h"
#include "Propagable.h"


// Propagates N-body systems and can also handle vessels and non-attracting bodies
class SystemPropagator
{
protected:
	StateVector* st_vector;
	LightStateVector* lst_vector;

public:


	void bind_to(Propagable* system)
	{
		this->st_vector = system->get_massful_states();
		this->lst_vector = system->get_light_states();
		init();
	}

	void bind_to(StateVector* s, LightStateVector* l)
	{
		this->st_vector = s;
		this->lst_vector = l;
		init();
	}


	virtual void init() {}
	// Propagates the system, including non-nbody bodies
	virtual void propagate(double dt) = 0;

	virtual ~SystemPropagator() = default;
};
