#include "VehiclePlumbing.h"
#include "../Vehicle.h"

float VehiclePlumbing::get_flow_rate(Pipe *p)
{
	// Each fluid gets a "flow dominance" ranking
	// that depends on density and ammount of fluid
	// We apply Bernoulli solved for velocity, and then multiply by surface


	return 0.0f;
}

void VehiclePlumbing::update_pipes(Vehicle* veh)
{
	// machines->fluid_preupdate();

	// First we solve junctions. They will query machines for pressure
	for(const auto& jnc : junctions)
	{

	}

	// Then we solve single pipes. They will query machines for pressure

	// Now we have every flow, make it real

	// machines->fluid_update();


}

void VehiclePlumbing::junction_flow_rate(PipeJunction& junction)
{
	size_t jsize = junction.pipes.size();

	// We impose the condition that sum(flow) = 0, and
	// as flow = ct * sqrt(deltaP), we can solve
	// 0 = sqrt(Pj - Pa) + sqrt(Pj - Pb) ...
	// Receiving tanks are added as -sqrt(Pa - Pj) as Pj > Pa
	// This setup divides our function into at max n parts,
	// with edges that we can determine: They are simply the
	// pressures sorted from high to low.

	// Sorted from highest pressure to lowest, safe to keep short-lived pointer
	std::vector<std::pair<Pipe*, float>> pipe_pressure;

	pipe_pressure.reserve(jsize);
	for(auto& pipe_id : junction.pipes)
	{
		Pipe* p = &pipes[pipe_id];
		float pr = p->ma->plumbing.get_pressure(p->port_a);
		pipe_pressure.emplace_back(p, pr);
	}

	// TODO: Obtain density by averaging or something
	float sqrt_density = 1.0f;

	auto evaluate = [pipe_pressure, jsize, sqrt_density](float x, size_t section, bool diff = false) -> std::pair<float, float>
	{
		// These later on will be read from the pipe
		float sum = 0.0f;
		float diff_sum = 0.0f;
		for(size_t i = 0; i < jsize; i++)
		{
			float sign = i > (jsize - 2 - section) ? -1.0f : 1.0f;
			float dP = sign * (pipe_pressure[i].second - x);
			float constant = pipe_pressure[i].first->surface * sqrt(2.0f) / sqrt_density;
			float radical = sqrt(dP);
			if(diff)
			{
				diff_sum -= constant / (2.0f * radical);
			}
			else
			{
				sum += sign * constant * radical;
			}
		}
		return std::make_pair(sum, diff_sum);
	};

	size_t solution_section;
	float x, fx;
	for(size_t i = 0; i < jsize - 1; i++)
	{
		float lP = pipe_pressure[i].second;
		float rP = pipe_pressure[i + 1].second;

		float left = evaluate(lP, i).first;
		float right = evaluate(rP, i).first;

		if(left * right < 0.0f)
		{
			solution_section = i;
			x = (lP + rP) / 2.0f;
			break;
		}
	}

	// Find an approximation of the solution, we use the Newton Rhapson method as
	// it's well behaved in this kind of function and converges VERY fast
	// TODO: Check if the closed form solution is faster AND
	// TODO:   if simply running the method on the whole piece-wise function
	// TODO:   converges (fast) too!
	for(size_t it = 0; it < 2; it++)
	{
		auto pair = evaluate(x, solution_section, false);
		fx = pair.first;
		float dfx = pair.second;
		x = x - fx / dfx;
	}

	// fx is very close to the pressure at the junction now
	for(size_t i = 0; i < jsize; i++)
	{
		float sign = i > (jsize - 2 - solution_section) ? -1.0f : 1.0f;
		float constant = pipe_pressure[i].first->surface * sqrt(2.0f) / sqrt_density;
		pipe_pressure[i].first->flow = sign * constant * sqrt(sign * (pipe_pressure[i].second - x));
	}


}
