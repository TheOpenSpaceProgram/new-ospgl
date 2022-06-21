#include "VehiclePlumbing.h"
#include "../Vehicle.h"

// A reasonable multiplier to prevent extreme flow velocities
// I don't know enough fluid mechanics as to determine a reasonable value
// so it's arbitrary, chosen to approximate real life rocket values
#define FLOW_MULTIPLIER 0.00002


VehiclePlumbing::VehiclePlumbing(Vehicle *in_vehicle)
{
	veh = in_vehicle;
}

std::vector<PlumbingMachine*> VehiclePlumbing::grid_aabb_check(glm::vec2 start, glm::vec2 end,
															   const std::vector<PlumbingMachine*>& ignore, bool expand)
{
	std::vector<PlumbingMachine*> out;
	std::vector<PlumbingMachine*> all_elems = get_all_elements();

	for(PlumbingMachine* pb : all_elems)
	{
		bool ignored = false;
		for(PlumbingMachine* m : ignore)
		{
			if(m == pb)
			{
				ignored = true;
				break;
			}
		}

		if(!ignored)
		{
			glm::ivec2 min = pb->editor_position;
			glm::ivec2 max = min + pb->get_size(expand);

			if (min.x < end.x && max.x > start.x && min.y < end.y && max.y > start.y)
			{
				out.emplace_back(pb);
			}
		}
	}


	return out;
}

glm::ivec4 VehiclePlumbing::get_plumbing_bounds()
{
	glm::ivec2 amin = glm::ivec2(INT_MAX, INT_MAX);
	glm::ivec2 amax = glm::ivec2(INT_MIN, INT_MIN);

	std::vector<PlumbingMachine*> all_elems = get_all_elements();
	bool any = false;
	for (PlumbingMachine* pb : all_elems)
	{
		glm::ivec2 min = pb->editor_position;
		glm::ivec2 max = pb->get_size(true) + min;

		amin = glm::min(amin, min);
		amax = glm::max(amax, max);
		any = true;
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
	// bottom as rockets are usually vertical
	glm::ivec4 bounds = get_plumbing_bounds();
	return glm::ivec2(bounds.x, bounds.y + bounds.w);


}

glm::ivec2 VehiclePlumbing::get_plumbing_size_of(Part* p)
{
	// min is always (0, 0) as that's the forced origin of the plumbing start positions
	glm::ivec2 max = glm::ivec2(INT_MIN, INT_MIN);
	bool found_any = false;

	for(const auto& pair : p->machines)
	{
		PlumbingMachine& pb = pair.second->plumbing;
		if(pb.has_lua_plumbing())
		{
			found_any = true;
			max = glm::max(max, pb.editor_position + pb.get_size());
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

std::vector<PlumbingMachine*> VehiclePlumbing::get_all_elements()
{
	std::vector<PlumbingMachine*> out;

	// Machines
	for(const Part* p : veh->parts)
	{
		for (const auto &pair : p->machines)
		{
			if (pair.second->plumbing.has_lua_plumbing())
			{
				// Safe to take a pointer, parts are in the heap
				out.push_back(&pair.second->plumbing);
			}
		}

		for(const auto &amac: p->attached_machines)
		{
			if(amac->plumbing.has_lua_plumbing())
			{
				out.push_back(&amac->plumbing);
			}
		}
	}

	return out;
}

int VehiclePlumbing::create_pipe()
{
	Pipe p = Pipe();
	pipes.push_back(p);
	return pipes.size() - 1;
}

void VehiclePlumbing::execute_flows(float dt)
{
	// We first normalize flows so that two equal flows from a machine
	// equal a single flow that splits in two (realistic)

	for(const FlowPath& path : fws)
	{
		float to_move = path.final_flow * dt;
		if(to_move == 0.0f)
			continue;
		StoredFluids buffer;
		Pipe& start = pipes[path.path.front().pipe];
		Pipe& end = pipes[path.path.back().pipe];
		// Bleed into the buffer
		FluidPort* from = path.path.front().backwards ? start.b : start.a;
		FluidPort* to = path.path.back().backwards ? end.a : end.b;

		logger->check(!from->is_flow_port && !to->is_flow_port, "A path starts/ends in a flow machine!");
		buffer.modify(from->in_machine->out_flow(from->id, to_move, true));
		float flow = (buffer.get_total_gas_mass() + buffer.get_total_liquid_mass()) / dt;
		start.flow += path.path.front().backwards ? flow : -flow;
		// This happens on pipes of length 1
		if(&end != &start)
		{
			end.flow += path.path.back().backwards ? flow : -flow;
		}
		// Travel the path if there's one
		if(path.path.size() > 1)
		{
			for (size_t i = 1; i < path.path.size() - 1; i++)
			{
				Pipe &p = pipes[path.path[i].pipe];
				p.flow += path.path[i].backwards ? flow : -flow;
				FluidPort* from_p = path.path[i].backwards ? p.b : p.a;
				FluidPort* to_p = path.path[i].backwards ? p.a : p.b;
				logger->check(from_p->is_flow_port && to_p->is_flow_port, "True machine in middle of path!");
				// TODO!
			}
		}

		to->in_machine->in_flow(to->id, buffer, true);

	}

}

// TODO: Quit condition if we enter a loop
void VehiclePlumbing::find_all_possible_paths()
{
	// Find a new unvisited pipe to start from that's connected to a real port
	for(size_t pipe_id = 0; pipe_id < pipes.size(); pipe_id++)
	{
		Pipe& pipe = pipes[pipe_id];
		if(!pipe.a->is_flow_port)
		{
			find_all_possible_paths_from(FlowStep(pipe_id, false, 0.0f));
		}

		if(!pipe.b->is_flow_port)
		{
			find_all_possible_paths_from(FlowStep(pipe_id, true, 0.0f));
		}
	}

}

void VehiclePlumbing::find_all_possible_paths_from(FlowStep start)
{
	// start.a/b contains a true port, so we just need to travel to port b/a now,
	// this is a tree search algorithm
	// We store the whole path to the open pipe so we can rebuild it later
	std::queue<std::vector<FlowStep>> open;
	std::vector<FlowStep> start_v;
	start_v.push_back(start);
	open.push(start_v);

	while(!open.empty())
	{
		std::vector<FlowStep> open_chain = open.front();
		size_t work = open_chain.back().pipe;
		bool backwards = open_chain.back().backwards;
		const Pipe* p = &pipes[work];
		const FluidPort* next = backwards ? p->a : p->b;

		open.pop();

		if(next->is_flow_port)
		{
			// Find all connected ports to this port and propagate in correct direction
			std::vector<FluidPort*> connected = next->in_machine->get_connected_ports(next->id);
			// Find the pipes connected as "a" to said ports, others are included in symmetric searches
			for(size_t pipe_id = 0; pipe_id < pipes.size(); pipe_id++)
			{
				Pipe& pipe = pipes[pipe_id];
				for(const FluidPort* port : connected)
				{
					std::vector<FlowStep> new_open;
					new_open.insert(new_open.begin(), open_chain.begin(), open_chain.end());
					bool found = false;
					if(pipe.a == port)
					{
						// Forward
						new_open.push_back(FlowStep(pipe_id, false, 0.0f));
						found = true;
					}
					else if(pipe.b == port)
					{
						// Backward
						new_open.push_back(FlowStep(pipe_id, true, 0.0f));
						found = true;
					}
					if(found)
					{
						open.push(new_open);
					}
				}
			}
		}
		else
		{
			FlowPath fpath;
			fpath.path = open_chain;
			calculate_delta_p(fpath);
			// We check that we don't end up where we started through a loop
			if(fpath.delta_P < 0.0f && next != (start.backwards ? pipes[start.pipe].b : pipes[start.pipe].a))
			{
				fws.push_back(fpath);
			}

		}
	}


}

void VehiclePlumbing::calculate_delta_p(FlowPath& fw)
{
	// It's guaranteed that all b ports are flow machines, so just follow them (except the end one)
	FluidPort* start = fw.path[0].backwards ? pipes[fw.path[0].pipe].b : pipes[fw.path[0].pipe].a;
	float start_P = start->in_machine->get_pressure(start->id);
	FluidPort* end = fw.path.back().backwards ? pipes[fw.path.back().pipe].a : pipes[fw.path.back().pipe].b;
	float end_P = end->in_machine->get_pressure(end->id);

	float P_drop = 0.0f;
	// We travel through the flow machines, observing the pressure drop between their ports
	// Note that the pressure drops happen to the start pressure (observe delta_P calculation)!
	for(size_t i = 0; i < fw.path.size() - 1; i++)
	{
		float cur_P = start_P - P_drop;
		FluidPort* in_port = fw.path[i].backwards ? pipes[fw.path[i].pipe].a : pipes[fw.path[i].pipe].b;
		FluidPort* out_port = fw.path[i + 1].backwards ? pipes[fw.path[i + 1].pipe].b : pipes[fw.path[i + 1].pipe].a;

		logger->check(in_port->in_machine == out_port->in_machine, "Something went wrong, machines don't match");
		logger->check(in_port->is_flow_port && out_port->is_flow_port, "Flow ports mismatched!");
		P_drop += in_port->in_machine->get_pressure_drop(in_port->id, out_port->id, cur_P);
	}

	for(size_t i = 0; i < fw.path.size(); i++)
	{
		FluidPort* in_port = fw.path[i].backwards ? pipes[fw.path[i].pipe].a : pipes[fw.path[i].pipe].b;
		FluidPort* out_port = fw.path[i].backwards ? pipes[fw.path[i].pipe].b : pipes[fw.path[i].pipe].a;
		float in_max = in_port->in_machine->get_maximum_flowrate(in_port->id);
		float out_max = out_port->in_machine->get_maximum_flowrate(out_port->id);
		if(in_max >= 0.0f || out_max >= 0.0f)
		{
			// We don't care about anything other than the pipe
			fw.rate_limiters.emplace_back(fw.path[i].pipe, glm::max(in_max, out_max));
		}
	}

	fw.delta_P = end_P - (start_P - P_drop);
	// If this is 0 or positive, it will be early discarded
	// End pressure must be lower than start pressure!
}

std::vector<size_t> VehiclePlumbing::find_forced_paths()
{
	std::vector<size_t> out;
	// We must go over every path and check which paths start and end in unique places, these
	// are the forced paths. The places are the port!
	std::set<FluidPort*> seen_start;
	std::set<FluidPort*> seen_end;

	std::set<FluidPort*> multiple_start;
	std::set<FluidPort*> multiple_end;

	for(const FlowPath& path : fws)
	{
		Pipe* front = &pipes[path.path.front().pipe];
		Pipe* back = &pipes[path.path.back().pipe];

		if(seen_start.count(path.path.front().backwards ? front->b : front->a) >= 1)
		{
			multiple_start.insert(front->a);
		}
		else
		{
			seen_start.insert(path.path.front().backwards ? front->b : front->a);
		}

		if(seen_end.count(path.path.back().backwards ? back->a : back->b) >= 1)
		{
			multiple_end.insert(path.path.back().backwards ? back->a : back->b);
		}
		else
		{
			seen_end.insert(path.path.back().backwards ? back->a : back->b);
		}
	}

	// Now, the elements which are not in multiple are unique
	for(size_t idx = 0; idx < fws.size(); idx++)
	{
		Pipe* front = &pipes[fws[idx].path.front().pipe];
		Pipe* back = &pipes[fws[idx].path.back().pipe];

		// If this is a unique path, it must happen and thus is forced.
		if(multiple_start.count(fws[idx].path.front().backwards ? front-> b: front->a) == 0 &&
		multiple_end.count(fws[idx].path.back().backwards ? back->a : back->b) == 0)
		{
			out.push_back(idx);
		}
	}

	return out;
}

bool VehiclePlumbing::remove_paths_not_compatible_with_forced()
{
	std::vector<size_t> forced = find_forced_paths();
	std::vector<size_t> to_remove;

	for(size_t i = 0; i < fws.size(); i++)
	{
		for(size_t f = 0; f < forced.size(); f++)
		{
			if(forced[f] != i)
			{
				if(!are_paths_compatible(fws[i], fws[forced[f]]))
				{
					to_remove.push_back(i);
				}
			}
		}
	}

	// Now we remove the given indices
	vector_remove_indices(fws, to_remove);

	return to_remove.empty();

}

bool VehiclePlumbing::are_paths_compatible(const VehiclePlumbing::FlowPath &a, const VehiclePlumbing::FlowPath &b)
{
	// We find shared points between the two paths
	for(size_t i = 0; i < a.path.size(); i++)
	{
		int pos = -1;
		for(size_t j = 0; j < b.path.size(); j++)
		{
			if(a.path[i].pipe == b.path[j].pipe)
			{
				pos = j;
				break;
			}
		}
		if(pos >= 0)
		{
			// Check that the traverse direction is the same, ie, we go forwards or backwards
			if(a.path[i].backwards != b.path[pos].backwards)
			{
				return false;
			}
		}
	}

	return true;
}

void VehiclePlumbing::reduce_to_forced_paths()
{
	size_t it = 0;
	// This removes all non compatible paths
	while(!remove_paths_not_compatible_with_forced())
	{
		if(it > 100)
		{
			logger->fatal("Reducing to forced paths is taking too long!");
		}
		it++;
	}

	// Now only forced paths remain, we have solved the system

}

int VehiclePlumbing::find_pipe_connected_to(FluidPort *port)
{
	// This is only used during vehicle editor so perfomance is not critical
	for(size_t i = 0; i < pipes.size(); i++)
	{
		if(pipes[i].a == port || pipes[i].b == port)
		{
			return i;
		}
	}

	return -1;
}

void VehiclePlumbing::update_pipes(float dt, Vehicle *in_vehicle)
{
	// Clear flows in pipes
	fws.clear();
	for(Pipe& p : pipes)
	{
		p.flow = 0.0f;
	}
	find_all_possible_paths();
	reduce_to_forced_paths();
	calculate_flowrates();
	execute_flows(dt);
}

void VehiclePlumbing::init()
{
	for(Pipe& p : pipes)
	{
		if(p.amachine)
		{
			p.a = p.amachine->get_port_by_id(p.aport);
			p.amachine = nullptr;
			p.aport = "";
		}

		if(p.bmachine)
		{
			p.b = p.bmachine->get_port_by_id(p.bport);
			p.bmachine = nullptr;
			p.bport = "";
		}
	}
}

// We must make sure we don't exceed a limiting flow across all flows on a single port!
void VehiclePlumbing::calculate_flowrates()
{
	// These ones have limited flowrates that MAY be shared between multiple FlowPaths
	std::unordered_multimap<size_t, std::pair<FlowPath&, float>> rate_limited;

	for(FlowPath& path : fws)
	{
		path.final_flow = sqrtf(-path.delta_P) * FLOW_MULTIPLIER;

		if(!path.rate_limiters.empty())
		{
			for(auto limiter : path.rate_limiters)
			{
				rate_limited.insert({limiter.first, {path, limiter.second}});
			}
		}
	}

	for(auto i = rate_limited.begin(); i != rate_limited.end();)
	{
		auto range = rate_limited.equal_range(i->first);
		float total_flow = 0.0f;
		for(auto d = range.first; d != range.second; d++)
		{
			total_flow += d->second.first.final_flow;
		}
		float reduce_factor = glm::min(range.first->second.second / total_flow, 1.0f);
		for(auto d = range.first; d != range.second; d++)
		{
			d->second.first.final_flow *= reduce_factor;
		}

		float new_total_flow = 0.0f;
		for(auto d = range.first; d != range.second; d++)
		{
			 new_total_flow += d->second.first.final_flow;
		}
		float n_factor = range.first->second.second / new_total_flow;
		i = range.second;
	}
}


void Pipe::invert()
{
	std::swap(a, b);
	std::reverse(waypoints.begin(), waypoints.end());
}

Pipe::Pipe()
{
	a = nullptr;
	b = nullptr;
	surface = 1.0f;
	flow = 0.0f;

}
