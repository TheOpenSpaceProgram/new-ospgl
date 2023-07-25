#include "VehicleMeta.h"
#include "Vehicle.h"


void VehicleMeta::remove_group(int64_t id)
{
	logger->check(id > 0 && id < group_names.size(), "Invalid range for group id");

	std::vector<Part*> in_group = find_parts_in_group(id);
	for(Part* p : in_group)
	{
		// Unassign group
		p->group_id = -1;
	}

	// Find all groups affected by the move
	for(int64_t i = id + 1; i < group_names.size(); i++)
	{
		in_group = find_parts_in_group(i);
		for(Part* p : in_group)
		{
			// Reduce the index as the element will be removed
			p->group_id--;
		}
	}

	group_names.erase(group_names.begin() + id);
}

void VehicleMeta::create_group(std::string name)
{
	group_names.push_back(name);
}

std::vector<Part*> VehicleMeta::find_parts_in_group(int64_t group)
{
	std::vector<Part*> out;
	for(Part* p : veh->parts)
	{
		// This automatically works with group = -1 (default group)
		if(p->group_id == group)
		{
			out.push_back(p);
		}
	}

	return out;
}

std::string VehicleMeta::get_group_name(int64_t id)
{
	if(id < 0)
	{
		return osp->game_database->get_string("core:default_group");
	}
	else
	{
		logger->check(id < (int64_t)group_names.size(), "Invalid group ID");
		return group_names[id];
	}
}

void VehicleMeta::set_controlled_machine(Machine* m)
{
	logger->check(m->in_part->vehicle == veh, "Tried to control a machine on another vehicle");
	controlled_machine = m->in_part_id;
	controlled_part = m->in_part->id;
}

std::shared_ptr<InputContext> VehicleMeta::get_input_ctx()
{
	Part* p = veh->get_part_by_id(controlled_part);
	if(!p)
		return nullptr;

	Machine* m = p->get_machine(controlled_machine);
	if(!m)
		return nullptr;

	return m->get_input_ctx();
}

static void find_symmetric_recurse(VehicleMeta* m, Piece* expand, std::vector<SymmetryInstance>& out, std::vector<Piece*>& seen)
{

	seen.push_back(expand);

	auto groups = m->find_symmetry_groups_containing(expand);
	for(size_t g : groups)
	{
		std::vector<SymmetryInstance> duplicates = m->symmetry_groups[g]->find_clones(expand, true);
		for(const auto& instance : duplicates)
		{
			bool already_added = false;
			for(auto& p : out)
			{
				if(p.p == instance.p)
				{
					if(!vector_contains(p.modes, instance.modes[0]))
					{
						p.modes.push_back(instance.modes[0]);
					}
					already_added = true;
					break;
				}
			}
			if(already_added)
				continue;

			out.push_back(instance);

			if(vector_contains(seen, instance.p))
				continue;

			find_symmetric_recurse(m, instance.p, out, seen);
		}

	}

}

std::vector<SymmetryInstance> VehicleMeta::find_symmetry_instances(Piece *p, bool include_p)
{
	std::vector<SymmetryInstance> out;
	std::vector<Piece*> seen;

	find_symmetric_recurse(this, p, out, seen);
	// TODO: This could be done directly in find_symmetry_recurse
	if(!include_p)
	{
		int f = -1;
		for(int i = 0; i < out.size(); i++)
		{
			if(out[i].p == p)
			{
				f = i;
				break;
			}
		}

		if(f >= 0)
		{
			out.erase(out.begin() + f);
		}
	}

	return out;
}

std::vector<size_t> VehicleMeta::find_symmetry_groups_containing(Piece *p)
{
	std::vector<size_t> out;
	for(size_t i = 0; i < symmetry_groups.size(); i++)
	{
		if(vector_contains(symmetry_groups[i]->all_in_symmetry, p))
			out.push_back(i);
	}
	return out;
}

void VehicleMeta::cleanup_symmetry_groups()
{
	for(auto it = symmetry_groups.begin(); it != symmetry_groups.end(); )
	{
		if((*it)->all_in_symmetry.empty())
		{
			it = symmetry_groups.erase(it);
		}
		else
		{
			it++;
		}
	}
}