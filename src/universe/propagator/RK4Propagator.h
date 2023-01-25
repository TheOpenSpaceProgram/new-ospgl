#pragma once
#include "SystemPropagator.h"

class RK4Propagator : public SystemPropagator
{
public:

	void init(Propagable* system) override {};
	// Propagates the system, including non-nbody bodies
	virtual void propagate(double dt) override;

	~RK4Propagator() override = default;

};

