#pragma once
#include <glm/glm.hpp>
#include <vector>


// Coordinates are GLOBAL, and are adjusted
// to the plotting frame when creating
// render-able (needs less points, so it's fast)
// Changing the plotting frame just requires
// re-doing the rendering stuff, not re-simulating
// so that's faster
struct Prediction
{

	// If true, then velocity for every single point
	// is stored, alognside position. This is enabled
	// for flight plan, as maneuvers need velocity.
	// The flight path prediction just needs position 
	// as it's a more coarse estimation, which doesn't 
	// allow maneuvers
	bool has_velocities;

	double t0;
	double tstep;
	double t1;

	std::vector<glm::dvec3> positions;
	std::vector<glm::dvec3> velocities;

	glm::dvec3 v_last;

	glm::dvec3 p0;
	glm::dvec3 v0;
	
	// nullptr if this is the end of the prediction
	Prediction* next;
};

// Allows prediction of an orbit in arbitrary 
// plotting frames 
// Vessel centerd plotting frames are special
// as the target vessel needs to be predicted too
class OrbitPredictor
{
public:

	// Every how much is a point added to the history
	// Maybe be bigger if timewarp is high!
	double history_interval = 10.0;

	// A hundred thousands points ~= 2.4Mb
	size_t max_history_points = 100000;

	// If >= 0, it indicates the point at which
	// history is being rewritten from
	int history_loop_point = -1;

	// Only stores positions, we cannot create maneuvers
	// in the past ;)
	std::vector<glm::dvec3> history;

	// Gets regenerated constantly while the vessel
	// is experiencing considerable velocity changes
	// and is automatically regenerated when we deviate
	// enough from the prediction
	// Relatively coarse prediction
	Prediction flight_path;

	// The planned prediction, including potential maneuvers
	// or just a higher quality, build-on-command plot
	Prediction planned;


	
	OrbitPredictor();
	~OrbitPredictor();
};

