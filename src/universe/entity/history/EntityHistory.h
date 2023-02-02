#pragma once
#include <vector>
#include <glm/glm.hpp>

struct HistoryPiece
{
	double dt;
	double t0;
	std::vector<glm::dvec3> pos;
};

class EntityHistory
{
private:
	double timer;
	double cur_dt;
	double t0;

public:
	std::vector<HistoryPiece> pieces;

	// Doesn't perform interpolation, not appropiate if dt is huge
	glm::dvec3 get_fast(double t);
	// Performs linear interpolation between points
	glm::dvec3 get(double t);

	double get_t0();
	double get_current_dt();
	// Negative value disables history generation
	// Upon resume, there may be holes in the history
	double set_dt(double val);

	// Potentially expensive call
	void forget(double new_t0);

	void update(double dt);
};
