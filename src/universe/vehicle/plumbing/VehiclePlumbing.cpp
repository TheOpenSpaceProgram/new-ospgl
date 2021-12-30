#include "VehiclePlumbing.h"
#include "../Vehicle.h"

// A reasonable multiplier to prevent extreme flow velocities
// I don't know enough fluid mechanics as to determine a reasonable value
// so it's arbitrary, chosen to approximate real life rocket values
#define FLOW_MULTIPLIER 0.0002

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
		float pr = pipe->mb->plumbing.get_pressure(pipe->port_b);
		pipe_pressure.emplace_back(pipe, pr);
	}

	std::sort(pipe_pressure.begin(), pipe_pressure.end(), [](const std::pair<Pipe*, float>& a, const std::pair<Pipe*, float>& b)
	{
		return a.second > b.second;
	});

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
			float dP = (pipe_pressure[i].second - x);
			float constant = pipe_pressure[i].first->surface * sqrt(2.0f) / sqrt_density;
			float radical = sqrt(glm::abs(dP));

			if(diff)
			{
				diff_sum -= constant / (2.0f * radical);
			}

			sum += sign * constant * radical;
		}
		return std::make_pair(sum, diff_sum);
	};

	size_t solution_section = 0;
	float x = 0.0f, fx;
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
		auto pair = evaluate(x, solution_section, true);
		fx = pair.first;
		float dfx = pair.second;
		// if the derivative is small, we are near constant and can early quit
		if(glm::abs(dfx) < 0.0001)
		{
			break;
		}
		x = x - fx / dfx;
	}

	// fx is very close to the pressure at the junction now
	for(size_t i = 0; i < jsize; i++)
	{
		float sign = i > (jsize - 2 - solution_section) ? -1.0f : 1.0f;
		float constant = pipe_pressure[i].first->surface * sqrt(2.0f) / sqrt_density;
		pipe_pressure[i].first->flow = sign * constant * sqrt(glm::abs(pipe_pressure[i].second - x)) * FLOW_MULTIPLIER;
	}



}

VehiclePlumbing::VehiclePlumbing(Vehicle *in_vehicle)
{
	veh = in_vehicle;
	pipe_id = 0;
	junction_id = 0;
}

std::vector<PlumbingElement> VehiclePlumbing::grid_aabb_check(glm::vec2 start, glm::vec2 end,
												 const std::vector<PlumbingElement>& ignore, bool expand)
{
	std::vector<PlumbingElement> out;

	for(const Part* p : veh->parts)
	{
		for (const auto &pair : p->machines)
		{
			bool ignored = false;
			for(PlumbingElement m : ignore)
			{
				if(m == pair.second)
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
					out.emplace_back(pair.second);
				}
			}
		}
	}

	// It's easier for junctions, note that we return pointers too
	for(PipeJunction& jnc : junctions)
	{
		bool ignored = false;
		for(PlumbingElement m : ignore)
		{
			if(m == &jnc)
			{
				ignored = true;
				break;
			}
		}

		glm::ivec2 min = jnc.pos;
		glm::ivec2 max = min + jnc.get_size(expand);

		if (min.x < end.x && max.x > start.x && min.y < end.y && max.y > start.y && !ignored)
		{
			out.emplace_back(&jnc);
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

std::vector<PlumbingElement> VehiclePlumbing::get_all_elements()
{
	std::vector<PlumbingElement> out;

	// Machines
	for(const Part* p : veh->parts)
	{
		for (const auto &pair : p->machines)
		{
			if (pair.second->plumbing.has_lua_plumbing())
			{
				out.emplace_back(pair.second);
			}
		}
	}

	// Junctions
	for(PipeJunction& jnc : junctions)
	{
		out.emplace_back(&jnc);
	}

	return out;
}

Pipe* VehiclePlumbing::create_pipe()
{
	size_t id = ++pipe_id;
	Pipe p = Pipe();
	p.id = id;

	pipes.push_back(p);

	// Rebuild the junctions as pointers may change in the vector
	rebuild_pipe_pointers();

	return &pipes[pipes.size() - 1];
}

PipeJunction* VehiclePlumbing::create_pipe_junction()
{
	size_t id = ++junction_id;
	PipeJunction j = PipeJunction();
	j.id = id;

	junctions.push_back(j);

	// Rebuild junctions in pipes as they may have changed pointer
	rebuild_junction_pointers();

	return &junctions[junctions.size() - 1];
}

Pipe* VehiclePlumbing::get_pipe(size_t id)
{
	for(Pipe& p : pipes)
	{
		if(p.id == id)
		{
			return &p;
		}
	}

	logger->fatal("Couldn't find pipe with id = {}", id);
	return nullptr;
}

PipeJunction* VehiclePlumbing::get_junction(size_t id)
{
	for(PipeJunction& p : junctions)
	{
		if(p.id == id)
		{
			return &p;
		}
	}

	logger->fatal("Couldn't find pipe junction with id = {}", id);
	return nullptr;
}

void VehiclePlumbing::remove_pipe(size_t id)
{
	bool found = false;
	for(size_t i = 0; i < pipes.size(); i++)
	{
		if(pipes[i].id == id)
		{
			pipes.erase(pipes.begin() + i);
			found = true;
			break;
		}
	}

	logger->check(found, "Couldn't find pipe with id= {} to remove", id);
	rebuild_pipe_pointers();

}

void VehiclePlumbing::remove_junction(size_t id)
{
	bool found = false;
	for(size_t i = 0; i < junctions.size(); i++)
	{
		if(junctions[i].id == id)
		{
			junctions.erase(junctions.begin() + i);
			found = true;
			break;
		}
	}

	logger->check(found, "Couldn't find junction with id= {} to remove", id);
	rebuild_junction_pointers();

}

void VehiclePlumbing::rebuild_junction_pointers()
{
	for(Pipe& p : pipes)
	{
		if(p.junction_id != 0)
		{
			PipeJunction* found = nullptr;
			for(PipeJunction& fj : junctions)
			{
				if(fj.id == p.junction_id)
				{
					found = &fj;
					break;
				}
			}

			logger->check(found != nullptr, "Couldn't find pipe junction with id = {}", p.junction_id);
			p.junction = found;
		}
	}
}

void VehiclePlumbing::rebuild_pipe_pointers()
{
	for(PipeJunction& jnc : junctions)
	{
		if(jnc.pipes.size() == 0)
		{
			// This is the case during vehicle loading
			jnc.pipes.resize(jnc.pipes_id.size());
		}

		for(size_t i = 0; i < jnc.pipes_id.size(); i++)
		{
			jnc.pipes[i] = get_pipe(jnc.pipes_id[i]);
		}
	}
}

std::vector<VehiclePlumbing::FlowPath> VehiclePlumbing::determine_flows()
{
	std::vector<FlowPath> out;

	return out;
}

void VehiclePlumbing::execute_flows(float dt, std::vector<FlowPath>& flows)
{

}


void VehiclePlumbing::find_all_possible_paths(std::vector<FlowPath> &fws)
{
	// Find a new unvisited pipe to start from that's connected to a real port
	for(const auto& pipe : pipes)
	{
		bool found_true = false;
		// We only need to check one side for simmetry
		for (const FluidPort &pa: pipe.a->in_machine->fluid_ports)
		{
			if (!pa.is_flow_port)
			{
				found_true = true;
				break;
			}
		}

		if (found_true)
		{
			find_all_possible_paths_from(fws, pipe);
			break;
		}
	}

}

void VehiclePlumbing::find_all_possible_paths_from(std::vector<FlowPath> &fws, const Pipe &start)
{
	// start.a contains a true port, so we just need to travel to port b now,
	// this is a tree search algorithm
	// We store the whole path to the open pipe so we can rebuild it later
	std::queue<std::vector<size_t>> open;
	std::vector<size_t> start_v;
	start_v.push_back(start.id);
	open.push(start_v);

	while(!open.empty())
	{
		std::vector<size_t> open_chain = open.front();
		size_t work = open_chain.back();
		const Pipe* p = get_pipe(work);
		open.pop();

		if(p->b->is_flow_port)
		{
			// Find all connected ports to this port and propagate
			std::vector<FluidPort*> connected = p->b->in_machine->get_connected_ports(p->b->id);
			// Find the pipes connected as "a" to said ports, others are included in symmetric searches
			for(const Pipe& pipe : pipes)
			{
				for(const FluidPort* port : connected)
				{
					if(pipe.a == port)
					{
						std::vector<size_t> new_open;
						new_open.insert(new_open.begin(), open_chain.begin(), open_chain.end());
						new_open.push_back(pipe.id);
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
			// Early discard
			if(fpath.delta_P > 0.0f)
			{
				fws.push_back(fpath);
			}
		}
	}


}

void VehiclePlumbing::calculate_delta_p(FlowPath& fws)
{
	// It's guaranteed that all b ports are flow machines, so just follow them (except the end one)
	FluidPort* start = get_pipe(fws.path[0])->a;
	float start_P = start->in_machine->get_pressure(start->id);
	FluidPort* end = get_pipe(fws.path[fws.path.size() - 1])->b;
	float end_P = end->in_machine->get_pressure(end->id);

	float P_drop = 0.0f;
	// We travel through the flow machines, observing the pressure drop between their ports
	// Note that the pressure drops happen to the start pressure (observe delta_P calculation)!
	for(size_t i = 0; i < fws.path.size() - 1; i++)
	{
		float cur_P = start_P - P_drop;
		FluidPort* in_port = get_pipe(fws.path[i])->b;
		FluidPort* out_port = get_pipe(fws.path[i + 1])->a;
		logger->check(in_port->in_machine == out_port->in_machine, "Something went wrong, machines don't match");
		P_drop += in_port->in_machine->get_pressure_drop(in_port->id, out_port->id, cur_P);
	}

	fws.delta_P = end_P - (start_P - P_drop);
	// If this is 0 or positive, it will be early discarded
	// End pressure must be lower than start pressure!
}

std::vector<size_t> VehiclePlumbing::find_forced_paths(std::vector<FlowPath>& fws)
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
		Pipe* front = get_pipe(path.path.front());
		Pipe* back = get_pipe(path.path.back());
		logger->check(front->a->is_flow_port == false, "Start must not be flow port");
		logger->check(front->b->is_flow_port == false, "End must not be flow port");

		if(seen_start.count(front->a) >= 1)
		{
			multiple_start.insert(front->a);
		}
		else
		{
			seen_start.insert(front->a);
		}

		if(seen_end.count(back->b) >= 1)
		{
			multiple_end.insert(back->b);
		}
		else
		{
			seen_end.insert(back->b);
		}
	}

	// Now, the elements which are not in multiple are unique
	for(size_t idx = 0; idx < fws.size(); idx++)
	{
		Pipe* front = get_pipe(fws[idx].path.front());
		Pipe* back = get_pipe(fws[idx].path.back());

		// If this is a unique path, it must happen and thus is forced.
		if(multiple_start.count(front->a) == 0 && multiple_end.count(back->b) == 0)
		{
			out.push_back(idx);
		}
	}

	return out;
}

bool VehiclePlumbing::remove_paths_not_compatible_with_forced(std::vector<FlowPath> &fws)
{
	std::vector<size_t> forced = find_forced_paths(fws);
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
		auto it = std::find(b.path.begin(), b.path.end(), a.path[i]);
		if(it != b.path.end())
		{
			// Check that the traverse direction is the same, ie, the next pipe is the same.
			// if it's not, we have found an incompatibility
			if((i == a.path.size() - 1 && it++ != b.path.end()) ||
					(i != a.path.size() - 1 && it++ == b.path.end()))
			{
				// Path a ends, but path b doesn't. It must mean they diverge
				// This prevents vector overflow
				return false;
			}

			// Now it's safe to check next index
			size_t next_a = a.path[i + 1];
			size_t next_b = *(it++);
			if(next_a != next_b)
			{
				return false;
			}
		}
	}

	return true;
}

void VehiclePlumbing::reduce_to_forced_paths(std::vector<FlowPath> &fws)
{
	size_t it = 0;
	// This removes all non compatible paths
	while(!remove_paths_not_compatible_with_forced(fws))
	{
		if(it > 100)
		{
			logger->fatal("Reducing to forced paths is taking too long!");
		}
		it++;
	}

	// Now only forced paths remain, we have solved the system

}


glm::ivec2 PipeJunction::get_size(bool extend, bool rotate) const
{
	size_t num_subs;
	if(get_port_number() <= 4)
	{
		num_subs = 1;
	}
	else
	{
		num_subs = ((get_port_number() - 5) / 2) + 2;
	}
	glm::ivec2 base = glm::ivec2(num_subs, 1);

	if(extend)
	{
		base += glm::ivec2(1, 1);
	}

	if(rotate && (rotation == 1 || rotation == 3))
	{
		std::swap(base.x, base.y);
	}

	return base;
}

void PipeJunction::add_pipe(Pipe* p)
{
	// Try to find a vacant slot
	for(size_t i = 0; i < pipes.size(); i++)
	{
		if(pipes[i] == nullptr && pipes_id[i] == 0xDEADBEEF)
		{
			pipes[i] = p;
			pipes_id[i] = p->id;
			return;
		}
	}
	// Otherwise add new pipe
	pipes.push_back(p);
	pipes_id.push_back(p->id);
}

glm::vec2 PipeJunction::get_port_position(const Pipe *p)
{
	// Port positions are a.lways in the same order, same as rendering
	float f = 1.0f;
	size_t port_count = get_port_number();
	int i = -1;
	for(size_t j = 0; j < pipes.size(); j++)
	{
		if(pipes[j] == p)
		{
			i = (int)j;
		}
	}
	logger->check(i != -1, "Couldn't find pipe with id = {}", p->id);

	glm::vec2 offset;
	if(port_count <= 4 || (i <= 3))
	{
		if(i == 0)
		{
			offset = glm::vec2(0.0f, -1.0f);
		}
		else if(i == 1)
		{
			offset = glm::vec2(-1.0f, 0.0f);
		}
		else if(i == 2)
		{
			offset = glm::vec2(0.0f, 1.0f);
		}
		else if(i == 3)
		{
			offset = glm::vec2(1.0f, 0.0f);
		}
	}
	else
	{
		// Algorithmic procedure for ports to the right
	}

	// Rotation
	offset += glm::vec2(0.5f);
	glm::ivec2 size = get_size(false, false);
	if(rotation == 1)
	{
		std::swap(offset.x, offset.y);
		offset.x = size.x - offset.x;
	}
	else if(rotation == 2)
	{
		offset.x = (float)size.x - offset.x;
		offset.y = (float)size.y - offset.y;
	}
	else if(rotation == 3)
	{
		std::swap(offset.x, offset.y);
		offset.y = (float)size.y - offset.y;
	}

	return offset * f + (glm::vec2)pos;

}

// This works even for vacant pipes!
size_t PipeJunction::get_port_id(const Pipe* p)
{
	int fid = -1;
	for(size_t i = 0; i < pipes.size(); i++)
	{
		if(pipes[i] == p)
		{
			fid = (int)i;
			break;
		}
	}

	logger->check(fid != -1, "Could not find pipe for port id");

	return (size_t)fid;

}

PlumbingElement::PlumbingElement(PipeJunction *junction)
{
	as_junction = junction;
	type = JUNCTION;
}

PlumbingElement::PlumbingElement()
{
	as_machine = nullptr;
	type = EMPTY;
}

PlumbingElement::PlumbingElement(Machine *machine)
{
	as_machine = machine;
	type = MACHINE;
}

// This operator== functions may unnecesarly check type as afterall
// there wont' be a machine and a pipe with the same memory index!
bool PlumbingElement::operator==(const Machine* m) const
{
	return type == MACHINE && as_machine == m;
}

bool PlumbingElement::operator==(const PipeJunction* jnc) const
{
	return type == JUNCTION && as_junction == jnc;
}

// TODO: This one is specially stupid
bool PlumbingElement::operator==(const PlumbingElement& j) const
{
	if(type == MACHINE)
	{
		return as_machine == j.as_machine && j.type == MACHINE;
	}
	else if(type == JUNCTION)
	{
		return as_junction == j.as_junction && j.type == JUNCTION;
	}

	return false;
}

glm::ivec2 PlumbingElement::get_size(bool expand, bool rotate)
{
	logger->check(type != EMPTY, "Tried to call get_size on an empty PlumbingElement");

	if(type == MACHINE)
	{
		return as_machine->plumbing.get_editor_size(expand, rotate);
	}
	else if(type == JUNCTION)
	{
		return as_junction->get_size(expand, rotate);
	}

	return glm::ivec2(0, 0);
}

glm::ivec2 PlumbingElement::get_pos()
{
	logger->check(type != EMPTY, "Tried to call get_pos on an empty PlumbingElement");

	if(type == MACHINE)
	{
		return as_machine->plumbing.editor_position;
	}
	else if(type == JUNCTION)
	{
		return as_junction->pos;
	}

	return glm::ivec2(0, 0);
}

void PlumbingElement::set_pos(glm::ivec2 pos)
{
	logger->check(type != EMPTY, "Tried to call set_pos on an empty PlumbingElement");

	if(type == MACHINE)
	{
		as_machine->plumbing.editor_position = pos;
	}
	else if(type == JUNCTION)
	{
		as_junction->pos = pos;
	}
}

int PlumbingElement::get_rotation()
{
	logger->check(type != EMPTY, "Tried to call get_rotation on an empty PlumbingElement");

	if(type == MACHINE)
	{
		return as_machine->plumbing.editor_rotation;
	}
	else if(type == JUNCTION)
	{
		return as_junction->rotation;
	}

	return 0;
}

void PlumbingElement::set_rotation(int value)
{
	logger->check(type != EMPTY, "Tried to call set_rotation on an empty PlumbingElement");

	if(type == MACHINE)
	{
		as_machine->plumbing.editor_rotation = value;
	}
	else if(type == JUNCTION)
	{
		as_junction->rotation = value;
	}

}

std::vector<std::pair<FluidPort, glm::vec2>> PlumbingElement::get_ports()
{
	logger->check(type != EMPTY, "Tried to call get_ports on an empty PlumbingElement");

	std::vector<std::pair<FluidPort, glm::vec2>> out;

	if(type == MACHINE)
	{
		for(const FluidPort& p : as_machine->plumbing.fluid_ports)
		{
			glm::vec2 pos = as_machine->plumbing.get_port_position(p.id);
			out.emplace_back(p, pos);
		}
	}
	else if(type == JUNCTION)
	{
		for(const Pipe* p : as_junction->pipes)
		{
			glm::vec2 pos = as_junction->get_port_position(p);
			FluidPort port = FluidPort();
			port.id = "__junction";
			port.numer_id = as_junction->get_port_id(p);
			port.gui_name = "Junction Port";
			port.marker = "";
			out.emplace_back(port, pos);
		}
	}

	return out;
}

void Pipe::invert()
{
	logger->check(junction == nullptr, "Cannot invert a pipe that goes to a junction as it must always be the end");
	std::swap(ma, mb);
	std::swap(port_a, port_b);
	std::reverse(waypoints.begin(), waypoints.end());
}

void Pipe::connect_junction(PipeJunction *jnc)
{
	junction = jnc;
	junction_id = jnc->id;
	ma = nullptr;
	port_a = "";

	jnc->add_pipe(this);
}

Pipe::Pipe()
{
	id = 0;
	ma = nullptr;
	mb = nullptr;
	junction = nullptr;
	junction_id = 0;
	port_a = "";
	port_b = "";

	surface = 1.0f;
	flow = 0.0f;

}
