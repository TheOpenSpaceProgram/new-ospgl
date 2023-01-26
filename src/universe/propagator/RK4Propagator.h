#pragma once
#include "SystemPropagator.h"

// Allows very good precision at big timesteps, but may be unnecesary at
// real-time timesteps!
class RK4Propagator : public SystemPropagator
{
private:
	size_t size;
	using SolVec = std::vector<std::pair<glm::dvec3, glm::dvec3>>;
	// First is x, second is v
	std::vector<std::pair<glm::dvec3, glm::dvec3>> u0;
	// First is x', second is v'
	std::vector<std::pair<glm::dvec3, glm::dvec3>> C1;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> C2;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> C3;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> C4;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> buffer1;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> buffer2;
	std::vector<std::pair<glm::dvec3, glm::dvec3>> buffer3;
	// Cache consistency may be improved this way
	std::vector<double> masses;

	void resize();

	void f(SolVec* target, const SolVec& eval_p);
	void set_buffer(SolVec* target, const SolVec& C, double dt);

public:

	// Propagates the system, including non-nbody bodies
	virtual void propagate(double dt) override;

	~RK4Propagator() override = default;

	void propagate_int(std::vector<glm::dvec3> *pos_target, std::vector<glm::dvec3> *vel_target,
					   const std::vector<glm::dvec3> &pos, const std::vector<glm::dvec3> &vel,
					   const std::vector<glm::dvec3> &acc, double dt);
};

