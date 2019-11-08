#pragma once
#include "SystemPropagator.h"

// Implements a variation of RK4 which only samples the
// solar system twice per time step, and interpolates 
// between samples. Appropiate for relatively small timesteps 
// and real-life-like systems as planetary variation is pretty 
// much linear over these scales
class RK4Interpolated : public SystemPropagator
{
private:

	struct Derivative
	{
		glm::dvec3 dx;
		glm::dvec3 dv;
	};


	PlanetarySystem* sys;
	PosVector t0_pos;
	PosVector t05_pos;
	PosVector t1_pos;
	MassVector masses;

	double t0, t1, tstep;

	Derivative sample(CartesianState s0, Derivative d, double dt, PosVector& vec);
	glm::dvec3 acceleration(glm::dvec3 p, PosVector& vec);

public:
	
	virtual void initialize(PlanetarySystem* system, size_t body_count) override;
	virtual void prepare(double t0, double tstep) override;
	virtual void propagate(Vessel* v) override;


};

