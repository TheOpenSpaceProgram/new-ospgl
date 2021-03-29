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
	for(auto& pipe : pipes)
	{
	}

}

std::vector<float> VehiclePlumbing::junction_flow_rate()
{
	std::vector<float> flows;
	size_t jsize = 4;

	// We impose the condition that sum(flow) = 0, and
	// as flow = ct * sqrt(deltaP), we can solve
	// 0 = sqrt(Pj - Pa) + sqrt(Pj - Pb) ...
	// Receiving tanks are added as -sqrt(Pa - Pj) as Pj > Pa
	// This setup divides our function into at max n parts,
	// with edges that we can determine: They are simply the
	// pressures sorted from high to low.
	// Then, the formula of every section can be generated as follows:

	// Sorted from higher to lowest
	std::vector<float> pressures;
	pressures.resize(jsize);
	pressures[0] = 10.0f;
	pressures[1] = 9.0f;
	pressures[2] = 5.0f;
	pressures[3] = 0.5f;

	auto evaluate = [pressures, jsize](float x, size_t section, bool diff = false) -> float
	{
		// These later on will be read from the pipe
		float sqrt_density = 1.0f;
		float sqrt_2 = sqrt(2.0f);
		float surface = 1.0f;
		float sum = 0.0f;
		for(size_t i = 0; i < jsize; i++)
		{
			float sign = i > (jsize - 2 - section) ? -1.0f : 1.0f;
			float dP = sign * (pressures[i] - x);
			float constant = surface * sqrt_2 / sqrt_density;
			float radical = sqrt(dP);
			if(diff)
			{
				sum -= constant / (2.0f * radical);
			}
			else
			{
				sum += sign * constant * radical;
			}
		}
		return sum;
	};

	size_t solution_section;
	float x;
	for(size_t i = 0; i < jsize - 1; i++)
	{
		float left = evaluate(pressures[i], i);
		float right = evaluate(pressures[i + 1], i);

		if(left * right < 0.0f)
		{
			solution_section = i;
			x = (pressures[i] + pressures[i + 1]) / 2.0f;
			break;
		}
	}

	// Find an approximation of the solution, we use the Newton Rhapson method as
	// it's well behaved in this kind of function and converges VERY fast
	for(size_t it = 0; it < 2; it++)
	{
		float fx = evaluate(x, solution_section, false);
		float dfx = evaluate(x, solution_section, true);
		x = x - fx / dfx;
	}


	logger->info("Solution: {}", x);



	return flows;
}
