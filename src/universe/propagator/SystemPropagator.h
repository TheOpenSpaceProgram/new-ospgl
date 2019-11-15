#pragma once
#include "../kepler/KeplerElements.h"
#include "../SystemElement.h"
#include "../UniverseDefinitions.h"

class PlanetarySystem;
class Vessel;

class SystemPropagator
{
public:


	virtual void initialize(PlanetarySystem* system, size_t body_count) = 0;
	virtual void prepare(double t0, double tstep, PosVector& out_pos) = 0;
	// Must return the index of the closest body, 0 being the star
	// (aka State index)
	virtual size_t propagate(Vessel* v) = 0;
};