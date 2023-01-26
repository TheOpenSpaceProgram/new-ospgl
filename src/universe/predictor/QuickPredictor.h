#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "FrameOfReference.h"
#include "../propagator/SystemPropagator.h"

class Prediction
{
private:
	// If positive, the prediction needs to be sent to the GPU starting
	// at index dirty in returned ptr from get_prediction_point
	int dirty;
public:

	virtual FrameOfReference get_ref() = 0;
	// Return true as long as there are more points to draw, set *ptr = &points
	// Will always be called to completion
	virtual bool get_prediction_points(std::vector<glm::dvec3>** ptr) = 0;
	// Return the TOTAL number of points in the prediction. May be bigger than the
	// actual value of points to pre-allocate space if you know num of points is going to grow
	virtual size_t get_num_points() = 0;
	// Return nullptr if points are already transformed into FrameOfReference
	virtual std::vector<CartesianState>* get_element_points() = 0;
	bool is_dirty(){ return dirty >= 0; }
	int get_dirty_level() {return dirty; }
	void unset_dirty() { dirty = -1; }
	void set_dirty(int depth){ dirty = depth;}

	Prediction()
	{
		dirty = -1;
	}
};

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
	FrameOfReference ref;
	std::vector<QuickPredictedInterval> intervals;
	size_t ptr;

	FrameOfReference get_ref() override { return ref;}
	// Return true as long as there are more points to draw
	bool get_prediction_points(std::vector<glm::dvec3>** ptr) override;
	std::vector<CartesianState>* get_element_points() override { return nullptr; };

	ShortTermPrediction()
	{
		ptr = 0;
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

public:

	// How much time can quick_predict run before it interrupts (approximate)
	// Negative means it won't be interrupted
	double quick_predict_timeout = 0.15;
	// How much in-game time can quick_predict run before it interrupts
	// Negative means it won't be interrupted
	// Make sure atleast one of them is positive
	double quick_predict_max_time = -1.0;

	// Continues an existing prediction (or starts a new one if pred contains no intervals)
	void sterm_predict(ShortTermPrediction* pred, glm::dvec3 pos = glm::dvec3(0, 0, 0),
					   glm::dvec3 vel = glm::dvec3(0, 0, 0));

	// Launches the worker thread
	void launch();

	void pause();
	void resume();

	explicit QuickPredictor(PlanetarySystem* sys);
	~QuickPredictor();
};

