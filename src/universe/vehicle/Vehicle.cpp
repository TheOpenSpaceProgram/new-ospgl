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

}

void Vehicle::editor_update(double dt)
{
	for(Part* part : parts)
	{
		part->editor_update(dt);
	}

}


std::vector<Vehicle*> Vehicle::physics_update(double dt)
{
	if(packed)	
	{
		return std::vector<Vehicle*>();
	}
	else
	{
		// Generate the gravity vector
		// glm::dvec3 pos = unpacked_veh.get_center_of_mass(); 
		glm::dvec3 pos = to_dvec3(root->get_global_transform().getOrigin());
		glm::dvec3 grav = in_universe->system.get_gravity_vector(pos, &in_universe->system.bullet_states);

		unpacked_veh.apply_gravity(to_btVector3(grav)); 
		auto n_vehicles = unpacked_veh.update();
		return n_vehicles;
	}	

}

void Vehicle::init(Universe* in_universe)
{
	init(&in_universe->lua_state);
	this->in_universe = in_universe;
}

void Vehicle::init(sol::state* lua_state)
{
	this->in_universe = nullptr;
	for(Part* part : parts)
	{
		part->init(lua_state, this);
	}
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

Piece* Vehicle::get_piece(int64_t id)
{
	auto it = id_to_piece.find(id);
	logger->check(it != id_to_piece.end(), "Wrong piece id ({})", id);
	return it->second;
}

Part* Vehicle::get_part(int64_t id)
{
	auto it = id_to_part.find(id);
	logger->check(it != id_to_part.end(), "Wrong piece id ({})", id);
	return it->second;
}

Vehicle::Vehicle() : unpacked_veh(this), packed_veh(this)
{
}

Vehicle::~Vehicle() 
{
	for(Part* p : parts)
	{
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
