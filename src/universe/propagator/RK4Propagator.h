#pragma once
#include "SystemPropagator.h"

// Allows very good precision at big timesteps, but may be unnecesary at
// real-time timesteps!
class RK4Propagator : public SystemPropagator
{
private:
	// Number of attracting bodies
	size_t size;
	// Number of non-attracting bodies
	size_t lsize;
	using SolVec = std::vector<std::pair<glm::dvec3, glm::dvec3>>;
	// First is x, second is v
	std::vector<std::pair<glm::dvec3, glm::dvec3>> u0;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> lu0; // < same for lstates
	// First is x', second is v'
	std::vector<std::pair<glm::dvec3, glm::dvec3>> C1;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> C2;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> C3;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> C4;
	// First is x, second is v
	std::vector<std::pair<glm::dvec3, glm::dvec3>> buffer1;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> buffer2;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> buffer3;

	// Same but for light states
	std::vector<std::pair<glm::dvec3, glm::dvec3>> lC1;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> lC2;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> lC3;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> lC4;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> lbuffer1;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> lbuffer2;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> lbuffer3;
	// Cache consistency may be improved this way
	std::vector<double> masses;

	void resize();

	// n-body
	void f(SolVec* target, const SolVec& eval_p);
	void set_buffer(SolVec* target, const SolVec& C, const SolVec& u0, double dt, size_t size);
	// light states
	void fl(SolVec* target, const SolVec& light_p, const SolVec& eval_p);

public:

	// Propagates the system, including non-nbody bodies
	virtual void propagate(double dt) override;

	~RK4Propagator() override = default;

	void propagate_int(std::vector<glm::dvec3> *pos_target, std::vector<glm::dvec3> *vel_target,
					   const std::vector<glm::dvec3> &pos, const std::vector<glm::dvec3> &vel,
					   const std::vector<glm::dvec3> &acc, double dt);
};

