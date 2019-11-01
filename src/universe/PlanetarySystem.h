#pragma once
#include <vector>
#include <cpptoml.h>
#include "../util/SerializeUtil.h"
#include "body/PlanetaryBody.h"
// A system holds a central star, origin of the coordinate system, and
// many planets orbiting it using keplerian orbits
class PlanetarySystem
{
public:
	
	double star_mass;

	// Guaranteed to be ordered so that the last planets to appear
	// are moons, or moons of moons (etc...)
	std::vector<PlanetaryBody> bodies;
	
	// Computes state of the whole system, including offsets, 
	// at a given time
	// We avoid allocating so you have to give a vector that's appropiately
	// sized (same as bodies.size())
	void compute_states(double t, std::vector<glm::dvec3>& out, double tol = 1.0e-8);

	// Computes state of planets which can have an effect on our vehicle, it ignores moons 
	// of planets whose SOI we are not touching, compunding them into a single mass (out_masses)
	// Keep in mind any out entries which are (0, 0) should be ignored!
	// (TODO)
	void compute_states_fast(double t, glm::dvec3 pos, std::vector<glm::dvec3>& out, std::vector<glm::dvec3>& out_masses, double tol = 1.0e-8);

	// Computes SOIs for all bodies at a given time, as it only depends on the 
	// semi-major axis it's not really important which t you choose.
	void compute_sois(double t);

	PlanetarySystem();
	~PlanetarySystem();
};


#include "PlanetarySystemSerializer.h"