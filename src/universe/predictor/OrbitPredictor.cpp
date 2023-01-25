#include "OrbitPredictor.h"



OrbitPredictor::OrbitPredictor(PlanetarySystem* nsys)
{
	this->sys = nsys;
}


OrbitPredictor::~OrbitPredictor()
= default;

void OrbitPredictor::quick_predict(ShortTermPrediction* pr, glm::dvec3 pos, glm::dvec3 vel, FrameOfReference ref)
{
	double start_time = glfwGetTime();
	std::vector<glm::dvec3> elements;
	if(pr->intervals.empty())
	{
		// Fetch the solar system state
		sys->lock.lock();

		sys->lock.unlock();
	}
	else
	{

	}
}
