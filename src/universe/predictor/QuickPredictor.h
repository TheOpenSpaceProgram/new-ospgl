#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "FrameOfReference.h"
#include "../propagator/SystemPropagator.h"
#include "PredictionDrawer.h"


// Doesn't support new system elements appearing / disappearing during simulation
// If such a thing happens, it will not be noticed by the prediction
struct QuickPredictedInterval
{
	double tstep;
	double t0;
	std::vector<glm::dvec3> pred;
	// Position of all elements at end of interval
	StateVector elems_at_end;
	// This is ALWAYS in global coordinates
	LightCartesianState pred_at_end;
};
// In this case, coordinates are relative to a Frame of Reference
// Meant to be quickly generated in real-time during burns or similar
// (although it's done in a thread, we aim for a prediction every few frames!)
class ShortTermPrediction : public Prediction
{
public:
	// Used to pre-allocate more points than actually needed in the GPU
	size_t min_points;
	size_t points;
	FrameOfReference ref;
	// Note: We always predict the whole thing at once, but this allows variable timestep
	std::vector<QuickPredictedInterval> intervals;
	size_t ptr;

	FrameOfReference get_ref() override { return ref;}
	// Return true as long as there are more points to draw
	bool get_prediction_points(std::vector<glm::dvec3>** ptr) override;
	size_t get_num_points() override;
	std::vector<CartesianState>* get_element_points() override { return nullptr; };

	ShortTermPrediction()
	{
		ptr = 0; points = 0; min_points = 0;
	}
};

// Runs a prediction for an orbit starting at current state of the solar system
// Each prediction will simulate the whole solar system, this could be greatly
// improved by caching the solar system state but that would require high memory usage
// Note: To save on memory velocities are not stored, instead they are calculated from position when needed
// This is performant because velocity is usually required at a few points, and not over the whole orbit
class QuickPredictor
{
private:
	PlanetarySystem* sys;
	PredictionDrawer* drawer;
	ShortTermPrediction pred;
	std::thread thread;

	glm::dvec3 pos, vel;
	std::mutex mtx;
	bool kill_thread;


	// Runs a short term prediction, called from the thread
	void sterm_predict(glm::dvec3 pos = glm::dvec3(0, 0, 0),
					   glm::dvec3 vel = glm::dvec3(0, 0, 0));

	// Returns once a timestep change is needed, with the new value
	// can also return if interrupt is needed, then return value will be negative
	double predict_interval(QuickPredictedInterval* inter, double& t, double sys_t0, double sys_t00, double tstep,
							double stime, FrameOfReference ref, size_t& it, SystemPropagator& prop,
							StateVector& st, LightStateVector& ls);

public:

	// How much time can quick_predict run before it interrupts (approximate)
	// Negative means it won't be interrupted
	double quick_predict_timeout = 0.15;
	// How much in-game time can quick_predict run before it interrupts
	// Negative means it won't be interrupted
	// Make sure atleast one of them is positive
	double quick_predict_max_time = -1.0;

	// If true, the prediction will fully stop (thread dies) once enough time is predicted
	bool max_time_stops;


	double get_scale();

	// For convenience of use as this is the most common use
	// The drawer is only created on first call to get_drawer, don't store the pointer!
	// It lives as long as we do, and we update it automatically
	PredictionDrawer* get_drawer();

	// Note: both of these are relatively expensive as the create / destroy the thread
	// TODO: Pause
	void launch();
	void stop();

	// Call every frame, an orbit will only be predicted once the thread interrupts
	void update(glm::dvec3 pos, glm::dvec3 vel);

	explicit QuickPredictor(PlanetarySystem* sys);
	~QuickPredictor();
};

