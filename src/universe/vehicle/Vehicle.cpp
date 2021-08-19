#include "Vehicle.h"

void Vehicle::unpack()
{
	logger->check(packed, "Tried to unpack an unpacked vehicle");
	
	packed = false;

	// Apply immediate velocity so physics don't start delayed
	WorldState st = packed_veh.get_world_state();
	double bdt = in_universe->PHYSICS_STEPSIZE; // TODO: in_universe->MAX_PHYSICS_STEPS * in_universe->PHYSICS_STEPSIZE ?
	st.cartesian.pos += st.cartesian.vel * bdt;
	//st.rotation *= st.angular_velocity * bdt;
	packed_veh.set_world_state(st);

	unpacked_veh.activate();	
}

void Vehicle::pack()
{
	logger->check(!packed, "Tried to pack a packed vehicle");

	packed = true;

	unpacked_veh.deactivate();

	packed_veh.calculate_com();
}

Piece* Vehicle::remove_piece(Piece* p)
{
	// TODO: Check links, attachments, etc... So we don't leak memory and create "orphan" stuff
	all_pieces.erase(std::remove(all_pieces.begin(), all_pieces.end(), p));
	if(p == root)
	{
		root = nullptr;
	}

	if(p->part && p->part->pieces["p_root"] == p)
	{
		// Remove the part too
		parts.erase(std::remove(parts.begin(), parts.end(), p->part));
	}

	return p;
}

void Vehicle::update(double dt)
{
	for(Part* part : parts)
	{
		part->pre_update(dt);
	}

	for(Part* part : parts)
	{
		part->update(dt);
	}

	plumbing.update_pipes(dt, this);

}

void Vehicle::editor_update(double dt)
{
	for(Part* part : parts)
	{
		part->editor_update(dt);
	}
}


void Vehicle::physics_update(double dt)
{
	if(!packed)
	{
		// Generate the gravity vector
		// glm::dvec3 pos = unpacked_veh.get_center_of_mass(); 
		glm::dvec3 pos = to_dvec3(root->get_global_transform().getOrigin());
		glm::dvec3 grav = in_universe->system.get_gravity_vector(pos, &in_universe->system.bullet_states);

		unpacked_veh.apply_gravity(to_btVector3(grav)); 
		unpacked_veh.update();
	}	

}

void Vehicle::init(Universe* universe)
{
	init(&universe->lua_state);
	this->in_universe = universe;
}

void Vehicle::init(sol::state* lua_state)
{
	this->in_universe = nullptr;

	remove_outdated();

	for(Part* part : parts)
	{
		part->init(lua_state, this);
	}
}

static bool find_machine(Machine* m, Vehicle* veh)
{
	if(!m->is_enabled())
		return false;

	for(auto& part : veh->parts)
	{
		for(auto& machine : part->machines)
		{
			if(machine.second == m)
			{
				return true;
			}
		}
	}

	return false;
}

static bool find_piece(Piece* p, Vehicle* veh)
{
	return p->in_vehicle == veh;
}

// TODO: This could be optimized by integrating it into UnpackedVehicle::handle_separation
void Vehicle::remove_outdated()
{
	// First of all, we will make missing pieces become nullptr
	// This also makes machines go missing
	for(Part* part : parts)
	{
		for(auto& pair : part->pieces)
		{
			if(!find_piece(pair.second, this))
			{
				pair.second = nullptr;
				for(auto& mpair : part->machines)
				{
					if(mpair.second->assigned_piece == pair.first)
					{
						mpair.second->piece_missing = true;
					}
				}
			}
		}
	}

	// Some machines may have gone missing
	for(auto it = wires.begin(); it != wires.end();)
	{
		if(!find_machine(it->first, this) || !find_machine(it->second, this))
		{
			it = wires.erase(it);
		}
		else
		{
			it++;
		}
	}

	// Some pipes might have been cut (TODO)
}

// Helper function for update_attachments
static std::pair<PieceAttachment, bool>* find_attachment(std::string marker, Piece* p)
{
	for(auto& pair : p->attachments)
	{
		if(pair.first.marker == marker)
		{
			return &pair;	
		}
	}	

	return nullptr;
}

void Vehicle::update_attachments()
{
	// First pass, clear used 
	for(Piece* p : all_pieces)
	{
		for(auto& pair : p->attachments)
		{
			pair.second = false;
		}
	}

	for(Piece* p : all_pieces)
	{
		if(p->attached_to)
		{
			auto* from = find_attachment(p->from_attachment, p);
			auto* to = find_attachment(p->to_attachment, p->attached_to);

			// This just sets the values if the pointer is not null, the 0 is useless
			from ? from->second = true : 0;
			to ? to->second = true : 0;
		}
	}
}


void Vehicle::sort()
{
	std::unordered_set<Piece*> open;

	open.insert(root);

	std::vector<Piece*> sorted;
	sorted.push_back(root);

	while (!open.empty())
	{
		std::unordered_set<Piece*> new_open;

		for (Piece* o : open)
		{

			for (Piece* p : all_pieces)
			{
				if (p->attached_to == o)
				{
					new_open.insert(p);
					sorted.push_back(p);
				}
			}
		}

		open = new_open;
		
	}

	logger->check(sorted.size() == all_pieces.size(), "Vehicle was sorted while some pieces were not attached!");

	all_pieces = sorted;
}


Vehicle::Vehicle() : unpacked_veh(this), packed_veh(this), plumbing(this)
{
}

Vehicle::~Vehicle() 
{
	for(Part* p : parts)
	{
		// We delete the parts as if they are in this vehicle it means
		// they are owned by this vehicle
		delete p;
	}

	for(Piece* p : all_pieces)
	{
		delete p;
	}

	// Ports are deleted by the machines
}


std::vector<Piece*> Vehicle::get_children_of(Piece* p)
{
	std::vector<Piece*> out;

	// TODO: Think of a better implementation
	// This one COULD stack overflow, and it's not particularly efficient
	for(Piece* np : all_pieces)
	{
		if(np->attached_to == p)
		{
			out.push_back(np);
		}
	}

	// Now find all children of every piece in out
	std::vector<Piece*> sub_children;
	for(Piece* child : out)
	{
		auto nchild = get_children_of(child);
		sub_children.insert(sub_children.end(), nchild.begin(), nchild.end());
	}

	out.insert(out.end(), sub_children.begin(), sub_children.end());

	return out;
}



std::vector<Piece*> Vehicle::get_attached_to(Piece* target) 
{
	std::vector<Piece*> out;

	for(Piece* p : all_pieces)
	{
		if(p->attached_to == target)
		{
			out.push_back(p);
		}
	}

	return out;
}

std::vector<Piece*> Vehicle::get_connected_to(Piece* target)
{
	std::vector<Piece*> attached = get_attached_to(target);
	if(target->attached_to != nullptr)
	{
		attached.push_back(target->attached_to);
	}

	return attached;
}

Piece* Vehicle::get_connected_with(Piece* p, const std::string& attachment_marker) 
{
	std::vector<Piece*> all = get_attached_to(p);
	// We aditionally check p to see if it's attached through attachment_marker
	all.push_back(p);
	for(Piece* subp : all)
	{
		if(subp == p)
		{
			// This is needed as it's from_attachment when p is attaching
			// and not to_attachment
			if(subp->from_attachment == attachment_marker)
			{
				return subp;
			}
		}
		else
		{
			if(subp->to_attachment == attachment_marker)
			{
				return subp;
			}
		}
		// * Note that attachments allow only one attached piece
	}

	return nullptr;
}

#include "VehicleLoader.h"

void GenericSerializer<Vehicle>::serialize(const Vehicle& what, cpptoml::table& target)
{
	VehicleSaver(target, what);
}


void GenericSerializer<Vehicle>::deserialize(Vehicle& to, const cpptoml::table& from) 
{
	VehicleLoader(from, to);
}