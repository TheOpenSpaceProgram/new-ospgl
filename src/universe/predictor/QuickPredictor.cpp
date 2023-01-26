#include "QuickPredictor.h"
// TODO: Configurable integrator?
#include "../propagator/RK4Propagator.h"


QuickPredictor::QuickPredictor(PlanetarySystem* nsys)
{
	this->sys = nsys;
}


QuickPredictor::~QuickPredictor() = default;

void QuickPredictor::sterm_predict(ShortTermPrediction *pr, glm::dvec3 pos, glm::dvec3 vel)
{
	double start_time = glfwGetTime();
	StateVector st;
	LightStateVector ls;
	LightCartesianState to_predict;
	QuickPredictedInterval interval;

	if(pr->intervals.empty())
	{
		// Fetch the solar system state
		sys->lock.lock();
		st = sys->states_now;
		sys->lock.unlock();
		// Start the predictor at given position and velocity
		to_predict.pos = pos;
		to_predict.vel = vel;
		interval.t0 = 0.0;
	}
	else
	{
		st = pr->intervals.back().elems_at_end;
		to_predict = pr->intervals.back().pred_at_end;
	}
	ls.push_back(to_predict);

	RK4Propagator prop;
	prop.bind_to(&st, &ls);

	constexpr size_t TIME_CHECK_INTERVAL = 5000;
	// TODO: Adaptive timestep
	double dt = 0.1;
	interval.tstep = dt;

	double r_time = 0.0;
	while(true)
	{
		for(size_t i = 0; i < TIME_CHECK_INTERVAL; i++)
		{
			prop.propagate(dt);
			// Save states
			auto state = st[pr->ref.center_id];
			auto body_pos = state.pos;
			// TODO: Rotations
			double body_rot = 0.0;
			auto npos = pr->ref.get_rel_pos(ls[0].pos, body_pos, body_rot);
			interval.pred.push_back(npos);
		}
		r_time += dt * TIME_CHECK_INTERVAL;

		double time = glfwGetTime();
		if((quick_predict_timeout > 0 && time - start_time > quick_predict_timeout) ||
		   (quick_predict_max_time > 0 && r_time > quick_predict_max_time))
		{
			break;
		}
	}

	interval.elems_at_end = st;
	interval.pred_at_end = ls[0];


}

bool ShortTermPrediction::get_prediction_points(std::vector<glm::dvec3>** to)
{
	if((size_t)ptr > intervals.size())
	{
		ptr = 0;
		return false;
	}
	else
	{
		*to = &intervals[ptr].pred;
		ptr++;
		return true;
	}
}
