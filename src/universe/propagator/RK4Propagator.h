#pragma once
#include "SystemPropagator.h"

class RK4Propagator : public SystemPropagator
{
private:

	PlanetarySystem* system;


public:

	void initialize(PlanetarySystem* system) override;
	// Propagates the system, including non-nbody bodies
	void propagate(StateVector& states, double dt) override;
	// Propagates a vessel / non-attracting body, must return index of closest body
	size_t propagate(CartesianState* state, const StateVector& states, double dt) override;

	~RK4Propagator() override = default;;

};

