#pragma once
#include "../kepler/KeplerElements.h"
#include "../element/SystemElement.h"
#include "../UniverseDefinitions.h"

class PlanetarySystem;

// Propagates N-body systems and can also handle vessels and non-attracting bodies
class SystemPropagator
{
public:


	virtual void initialize(PlanetarySystem* system) = 0;
	// Propagates the system, including non-nbody bodies
	virtual void propagate(StateVector& states, double dt) = 0;
	// Propagates a vessel / non-attracting body, must return index of closest body
	virtual size_t propagate(CartesianState* state, const StateVector& states, double dt) = 0;

	virtual ~SystemPropagator() = default;
};
