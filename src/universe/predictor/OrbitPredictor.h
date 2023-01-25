#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "FrameOfReference.h"

// Doesn't support new system elements appearing / disappearing during simulation
// If such a thing happens, it will not be noticed by the prediction
struct QuickPredictedInterval
{
	double tstep;
	double t0;
	std::vector<LightCartesianState> pred;
	// Position of all elements at end of interval
	std::vector<CartesianState> elems_at_end;
};
// In this case, coordinates are relative to a Frame of Reference
// Meant to be quickly generated in real-time during burns or similar
// (although it's done in a thread, we aim for a prediction every few frames!)
struct ShortTermPrediction
{
	FrameOfReference ref;
	std::vector<QuickPredictedInterval> intervals;
};

// Runs a prediction for an orbit starting at current state of the solar system
// Each prediction will simulate the whole solar system, this could be greatly
// improved by caching the solar system state but that would require high memory usage
class OrbitPredictor
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

	void quick_predict(ShortTermPrediction* pred, glm::dvec3 pos, glm::dvec3 vel, FrameOfReference ref);

	explicit OrbitPredictor(PlanetarySystem* sys);
	~OrbitPredictor();
};

