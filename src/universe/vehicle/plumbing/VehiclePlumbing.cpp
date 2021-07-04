#include "VehiclePlumbing.h"
#include "../Vehicle.h"

void VehiclePlumbing::update_pipes(float dt, Vehicle* veh)
{
	// machines->fluid_preupdate();

	// First we solve junctions. They will query machines for pressure
	for(const auto& jnc : junctions)
	{
		// First we do a pass to see how much can be accepted by the receivers
		// This is to prevent "sucking" too much from the suppliers and then
		// having to give back
		// We assume gases can be accepted in infinite ammount as they are compressible
		float available_liquid_volume = 0.0f;
		float intake_liquid_volume = 0.0f;
		junction_flow_rate(jnc, dt);

		for(Pipe* p : jnc.pipes)
		{
			intake_liquid_volume += p->flow;
			if(p->flow > 0.0f)
			{
				//available_liquid_volume += p->ma->plumbing.get_free_volume();
			}
		}

		StoredFluids in;
		for(Pipe* p : jnc.pipes)
		{
			if(p->flow > 0.0f)
			{
				//in.modify(p->ma->plumbing.out_flow(p->port_a, p->flow));
			}
		}
		// It may happen that "in" is not enough as to satisfy all pipes
	}

	// Then we solve single pipes. They will query machines for pressure

	// machines->fluid_update();


}

void VehiclePlumbing::junction_flow_rate(const PipeJunction& junction, float dt)
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
	for(auto& pipe : junction.pipes)
	{
		float pr = pipe->ma->plumbing.get_pressure(pipe->port_a);
		pipe_pressure.emplace_back(pipe, pr);
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
		// We multiply by dt to obtain flow in m^3 instead of flow rate
		pipe_pressure[i].first->flow = sign * constant * sqrt(sign * (pipe_pressure[i].second - x)) * dt;
	}


}

VehiclePlumbing::VehiclePlumbing(Vehicle *in_vehicle)
{
	veh = in_vehicle;
}

std::vector<Machine*> VehiclePlumbing::grid_aabb_check(glm::vec2 start, glm::vec2 end,
												 const std::vector<Machine*>& ignore, bool expand) const
{
	std::vector<Machine*> out;

	for(const Part* p : veh->parts)
	{
		for (const auto &pair : p->machines)
		{
			bool ignored = false;
			for(Machine* m : ignore)
			{
				if(pair.second == m)
				{
					ignored = true;
					break;
				}
			}

			if(pair.second->plumbing.has_lua_plumbing() && !ignored)
			{
				glm::ivec2 min = pair.second->plumbing.editor_position;
				glm::ivec2 max = min + pair.second->plumbing.get_editor_size(expand);

				if (min.x < end.x && max.x > start.x && min.y < end.y && max.y > start.y)
				{
					out.push_back(pair.second);
				}
			}
		}
	}

	return out;
}

glm::ivec4 VehiclePlumbing::get_plumbing_bounds()
{
	glm::ivec2 amin = glm::ivec2(INT_MAX, INT_MAX);
	glm::ivec2 amax = glm::ivec2(INT_MIN, INT_MIN);

	bool any = false;
	for(const Part* p : veh->parts)
	{
		for (const auto &pair : p->machines)
		{
			if (pair.second->plumbing.has_lua_plumbing())
			{
				glm::ivec2 min = pair.second->plumbing.editor_position;
				glm::ivec2 max = min + pair.second->plumbing.get_editor_size(true);

				amin = glm::min(amin, min);
				amax = glm::max(amax, max);
				any = true;
			}
		}
	}

	if(any)
	{
		return glm::ivec4(amin.x, amin.y, amax.x - amin.x, amax.y - amin.y);
	}
	else
	{
		return glm::ivec4(0, 0, 0, 0);
	}
}

glm::ivec2 VehiclePlumbing::find_free_space(glm::ivec2 size)
{
	// We first do a binary search in the currently used space

	// If that cannot be found, return outside of used space, to the
	// bottom right
	glm::ivec4 bounds = get_plumbing_bounds();
	return glm::ivec2(bounds.x + bounds.z, bounds.y + bounds.w);


}

glm::ivec2 VehiclePlumbing::get_plumbing_size_of(Part* p)
{
	// min is always (0, 0) as that's the forced origin of the plumbing start positions
	glm::ivec2 max = glm::ivec2(INT_MIN, INT_MIN);
	bool found_any = false;

	for(const auto& pair : p->machines)
	{
		MachinePlumbing& pb = pair.second->plumbing;
		if(pb.has_lua_plumbing())
		{
			found_any = true;
			max = glm::max(max, pb.editor_position + pb.get_editor_size());
		}
	}

	if(found_any)
	{
		return max;
	}
	else
	{
		return glm::ivec2(0, 0);
	}

}

