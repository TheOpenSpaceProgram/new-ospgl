#pragma once
#include "../kepler/KeplerElements.h"
#include "../SystemElement.h"

class PlanetarySystem;
class Vessel;

class SystemPropagator
{
public:

	using StateVector = std::vector<CartesianState>;
	using PosVector = std::vector<glm::dvec3>;
	using ElementVector = std::vector<SystemElement>;
	using MassVector = std::vector<double>;

	virtual void initialize(PlanetarySystem* system, size_t body_count) = 0;
	virtual void prepare(double t0, double tstep) = 0;
	virtual void propagate(Vessel* v) = 0;

};