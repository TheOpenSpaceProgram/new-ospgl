#pragma once
#include "../UniverseDefinitions.h"
#include "Propagable.h"


// Propagates N-body systems and can also handle vessels and non-attracting bodies
class SystemPropagator
{
private:
	Propagable* prop;
	StateVector* st_vector;
	LightStateVector* lst_vector;

public:


	void bind_to(Propagable* system)
	{
		this->prop = system;
	}

	virtual void init(Propagable* system) = 0;
	// Propagates the system, including non-nbody bodies
	virtual void propagate(double dt) = 0;

	virtual ~SystemPropagator() = default;
};
