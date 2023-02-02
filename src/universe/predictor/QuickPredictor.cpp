#include "QuickPredictor.h"
// TODO: Configurable integrator?
#include "../propagator/RK4Propagator.h"


QuickPredictor::QuickPredictor(PlanetarySystem* nsys)
{
	this->sys = nsys;
	this->drawer = nullptr;
}


QuickPredictor::~QuickPredictor()
{
	stop();
	delete drawer;
};

void QuickPredictor::sterm_predict(glm::dvec3 spos, glm::dvec3 svel)
{
	double start_time = glfwGetTime();
	StateVector st;
	LightStateVector ls;
	TrajectoryVector tv;
	tv.push_back(nullptr);
	LightCartesianState to_predict;

	// Fetch the solar system state
	sys->lock.lock();
	double t0 = sys->t;
	double t00 = sys->t0;
	st = sys->states_now;
	sys->lock.unlock();
	// Start the predictor at given position and velocity
	to_predict.pos = spos;
	to_predict.vel = svel;
	ls.push_back(to_predict);
	// Fetch prediction
	pred.lock.lock();
	FrameOfReference pred_ref = pred.ref;
	pred.lock.unlock();

	RK4Propagator prop;
	prop.bind_to(&st, &ls, &tv);

	size_t it = 0;

	std::vector<QuickPredictedInterval> intervals;
	intervals.emplace_back();

	// TODO: Adaptive timestep
	double tstep = 1.0;
	double t = 0.0;
	double stime = glfwGetTime();
	while(true)
	{
		tstep = predict_interval(&intervals[intervals.size() - 1], t, t0, t00, tstep, stime,
								 pred_ref, it, prop, st, ls);
		if(tstep > 0)
			intervals.emplace_back();
		else
			break;
	}



	pred.lock.lock();
	pred.intervals = std::move(intervals);
	pred.set_dirty(0);
	for(size_t i = 0; i < pred.intervals.size(); i++)
	{
		pred.points = pred.points + pred.intervals[i].pred.size();
	}
	pred.lock.unlock();

}

PredictionDrawer *QuickPredictor::get_drawer()
{
	if (drawer == nullptr)
	{
		drawer = new PredictionDrawer((Prediction*)&pred, get_scale());
	}
	return drawer;
}

double QuickPredictor::get_scale()
{
	// TODO: Heuristic based on length of prediction, speed, etc...
	return 1.0;
}

void QuickPredictor::stop()
{
	kill_thread = true;
	if(thread.joinable())
		thread.join();
}

void QuickPredictor::launch()
{
	kill_thread = false;

	// Thread is already running?
	if(thread.joinable())
		return;

	thread = std::thread([this]()
	{
		set_this_thread_name("quick_pred");
		while(true)
		{
			this->mtx.lock();
			if(this->kill_thread)
				return;
			glm::dvec3 spos = this->pos;
			glm::dvec3 svel = this->vel;
			this->mtx.unlock();
			sterm_predict(spos, svel);
		}
	});
}

double
QuickPredictor::predict_interval(QuickPredictedInterval* inter, double& t, double sys_t0, double sys_t00, double tstep,
								 double stime, FrameOfReference ref, size_t& it, SystemPropagator& prop,
								 StateVector& st, LightStateVector& ls)
{
	const size_t TIME_CHECK_INTERVAL = 10000;
	// How much in-game time between drawn points?
	// Useful to avoid sending massive ammount of points to the GPU
	const double SAVE_INTERVAL = 10.0;
	size_t save_it = 0;
	size_t save_interval = SAVE_INTERVAL / tstep;
	while(true)
	{
		for(; it < TIME_CHECK_INTERVAL; it++)
		{
			prop.propagate(tstep);
			save_it++;
			if(save_it > save_interval)
			{
				// Save states
				auto state = st[ref.center_id];
				auto body_pos = state.pos;
				// TODO: Rotations using sys_t0 and sys_t00
				double body_rot = 0.0;
				auto npos = ref.get_rel_pos(ls[0].pos, body_pos, body_rot);
				inter->pred.push_back(npos);
				save_it = 0;
			}
			t += tstep;
		}

		it = 0;
		double time = glfwGetTime();
		if((quick_predict_timeout > 0 && time - stime > quick_predict_timeout) ||
		   (quick_predict_max_time > 0 && t > quick_predict_max_time))
		{
			// Interrupt
			return -1.0;
		}
	}
}

void QuickPredictor::update(glm::dvec3 npos, glm::dvec3 nvel)
{
	mtx.lock();
	this->pos = npos;
	this->vel = nvel;
	mtx.unlock();
	if(drawer)
	{
		drawer->update();
	}
}

void QuickPredictor::on_add_to_renderer()
{
	// Create the drawer if not present
	get_drawer();
}

void QuickPredictor::forward_pass(CameraUniforms &cu, bool is_env_map)
{
	// Drawer will always be present if this is called
	drawer->forward_pass(cu);
}


bool ShortTermPrediction::get_prediction_points(std::vector<glm::dvec3>** to)
{
	if((size_t)ptr >= intervals.size())
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

size_t ShortTermPrediction::get_num_points()
{
	return std::max(min_points, points);
}
