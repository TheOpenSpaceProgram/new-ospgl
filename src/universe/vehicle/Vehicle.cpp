#include "Vehicle.h"

void Vehicle::unpack()
{
	logger->check(packed, "Tried to unpack an unpacked vehicle");
	
	packed = false;

	// Apply immediate velocity so physics don't start delayed
	WorldState st = packed_veh.get_world_state();
	double bdt = in_universe->PHYSICS_STEPSIZE; // TODO: in_universe->MAX_PHYSICS_STEPS * in_universe->PHYSICS_STEPSIZE ?
	st.pos += st.vel * bdt;
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


void Vehicle::physics_update(double dt)
{
	if(!packed)
	{
		// Generate the gravity vector
		// glm::dvec3 pos = unpacked_veh.get_center_of_mass(); 
		glm::dvec3 pos = to_dvec3(root->get_global_transform().getOrigin());
		glm::dvec3 grav = in_universe->system.get_gravity_vector(pos, true);

		unpacked_veh.apply_gravity(to_btVector3(grav)); 
		unpacked_veh.update();
	}

	// TODO: Maybe add a pre_plumbing update or sort this stuff?

	plumbing.update_pipes(dt, this);

	for(Part* part : parts)
	{
		part->physics_update(dt);
	}

}

void Vehicle::init(Universe* universe, Entity* in_entity)
{
	init(&universe->lua_state);
	this->in_universe = universe;
	this->in_entity = in_entity;
}

void Vehicle::init(sol::state* lua_state)
{
	this->in_universe = nullptr;

	remove_outdated();

	for(Part* part : parts)
	{
		part->init(lua_state, this);
	}

	plumbing.init();
}

static bool find_machine(Machine* m, Vehicle* veh)
{
	if(!m->is_enabled())
		return false;

	for(auto& part : veh->parts)
	{
		for(auto& machine : part->get_all_machines())
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

	// Some logical connections my have been cut
	for(auto& pair : logical_groups)
	{
		LogicalGroup* g = pair.second;
		for (auto it = g->connections.begin(); it != g->connections.end();)
		{
			if (!find_machine(it->first, this) || !find_machine(it->second, this))
			{
				it = g->connections.erase(it);
			}
			else
			{
				it++;
			}
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
	emit_event("on_sort");

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
	emit_event("after_sort");
}


Vehicle::Vehicle() : unpacked_veh(this), packed_veh(this), plumbing(this), meta(this)
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
	VehicleLoader(from, to, false);
}

Part* Vehicle::get_part_by_id(int64_t id)
{
	auto it = id_to_part.find(id);
	if(it == id_to_part.end())
		return nullptr;
	else
		return it->second;
}

Piece* Vehicle::get_piece_by_id(int64_t id)
{
	auto it = id_to_piece.find(id);
	if(it == id_to_piece.end())
		return nullptr;
	else
		return it->second;
}

std::pair<glm::dvec3, glm::dvec3> Vehicle::get_bounds()
{
	auto min = glm::dvec3(HUGE_VAL);
	auto max = glm::dvec3(-HUGE_VAL);

	for(Piece* p : all_pieces)
	{
		// aabb_min and aabb_max are in collider coordinates, move them to piece
		// coordinates and then apply the in_vehicle_matrix
		// TODO: This may not be correct, appears to be, but get_in_vehicle_matrix requires a bit of mathematical
		// thinking to get it right
		auto tform = p->get_in_vehicle_matrix() * glm::inverse(p->collider_offset);
		glm::dvec3 trans_min = glm::dvec3(tform * glm::dvec4(p->piece_prototype->aabb_min, 1.0));
		glm::dvec3 trans_max = glm::dvec3(tform * glm::dvec4(p->piece_prototype->aabb_max, 1.0));
		// Due to transforms, anything may become max or min
		min = glm::min(min, trans_min);
		min = glm::min(min, trans_max);
		max = glm::max(max, trans_min);
		max = glm::max(max, trans_max);
	}

	return std::make_pair(min, max);
}

Piece* Vehicle::duplicate(Piece *p, sol::state* st, int64_t* opiece_id, int64_t* opart_id)
{
	if(st == nullptr)
	{
		st = &osp->universe->lua_state;
	}

	logger->check(p->in_vehicle == this);

	std::unordered_map<Piece*, int> piece_to_index;

	std::vector<Piece*> all = get_children_of(p);
	all.insert(all.begin(), p);

	for(int i = 0; i < all.size(); i++)
	{
		piece_to_index[all[i]] = i;
	}

	// Basic clone of all pieces
	std::vector<Piece*> new_all;
	std::vector<Part*> new_parts;
	std::vector<Part*> seen_parts;
	for(Piece* a : all)
	{
		Piece* new_a = clone_piece(a, seen_parts, new_parts, opiece_id, opart_id);
		new_all.push_back(new_a);
	}

	// Assign links, we exploit the fact that all and new_all vectors share
	// the same indices
	for(int i = 0; i < all.size(); i++)
	{
		// Skip the start piece because it's attached to an external piece (or not)
		if(i > 0)
		{
			int target = piece_to_index[all[i]->attached_to];
			new_all[i]->attached_to = new_all[target];
			new_all[i]->from_attachment = all[i]->from_attachment;
			new_all[i]->to_attachment = all[i]->to_attachment;
		}
		// We do copy link type of start piece if it was attached
		if(all[i]->attached_to != nullptr)
		{
			if (all[i]->welded)
			{
				new_all[i]->welded = true;
			}
			else
			{
				// Copy the link fully
				new_all[i]->link_from = all[i]->link_from;
				new_all[i]->link_to = all[i]->link_to;
				new_all[i]->link_rot = all[i]->link_rot;
				logger->check(false, "NOT IMPLEMENTED - Did you check link before attach?");
			}
		}
	}


	// Add all to the vehicle
	all_pieces.insert(all_pieces.end(), new_all.begin(), new_all.end());
	parts.insert(parts.end(), new_parts.begin(), new_parts.end());

	// Assign IDs
	for(Part* pa : new_parts)
	{
		id_to_part[pa->id] = pa;
	}
	for(Piece* pi : new_all)
	{
		id_to_piece[pi->id] = pi;
	}
	// Initialize the parts
	for(Part* pa : new_parts)
	{
		pa->init(st, this);
	}

	return new_all[0];
}

Piece* Vehicle::clone_piece(Piece *p, std::vector<Part *> &seen_parts, std::vector<Part*> &created_parts,
							int64_t* opiece_id, int64_t* opart_id)
{
	if(opiece_id == nullptr)
	{
		opiece_id = &osp->universe->piece_uid;
		opart_id = &osp->universe->part_uid;
	}

	Piece* piece = new Piece(p->part_prototype, p->name_in_part);
	piece->packed_tform = p->packed_tform;

	Part* part = nullptr;
	if(p->part != nullptr)
	{
		for (size_t i = 0; i < seen_parts.size(); i++)
		{
			if (seen_parts[i] == p->part)
			{
				part = created_parts[i];
			}
		}

		// If we havent already, clone the part
		if(part == nullptr)
		{
			// TODO: Pass the table too?
			part = new Part(p->part_prototype, nullptr);
			(*opart_id)++;
			part->id = *opart_id;
			seen_parts.push_back(p->part);
			created_parts.push_back(part);
		}
	}
	piece->part = part;
	piece->in_vehicle = this;
	(*opiece_id)++;
	piece->id = *opiece_id;
	if(part)
	{
		part->pieces[p->name_in_part] = piece;
	}

	return piece;
}

void Vehicle::move_piece(Piece *p, glm::dvec3 new_pos, glm::dquat new_rot, const std::string& marker)
{
	// TODO: Make this work in unpacked? May rarely be used tho
	logger->check(is_packed(), "Unable to move pieces while unpacked");

	std::vector<Piece*> children = get_children_of(p);
	glm::dmat4 tform_0 = p->get_graphics_matrix();

	glm::dmat4 tform_marker;
	if(!marker.empty())
	{
		tform_marker = p->get_marker_transform(marker);
	}

	glm::dmat4 tform_new = glm::translate(glm::dmat4(1.0), new_pos);
	tform_new = tform_new * glm::toMat4(new_rot) * glm::inverse(tform_marker);
	p->packed_tform = to_btTransform(tform_new);

	for(Piece* child : children)
	{
		glm::dmat4 relative = glm::inverse(tform_0) * child->get_graphics_matrix() * child->collider_offset;
		glm::dmat4 final = tform_new * relative;
		child->packed_tform = to_btTransform(final);
	}

}

void Vehicle::move_piece_mat(Piece *p, glm::dmat4 new_tform)
{
	// TODO: Make this work in unpacked? May rarely be used tho
	logger->check(is_packed(), "Unable to move pieces while unpacked");

	std::vector<Piece*> children = get_children_of(p);
	glm::dmat4 tform_0 = p->get_graphics_matrix();

	p->packed_tform = to_btTransform(new_tform * p->collider_offset);

	for(Piece* child : children)
	{
		glm::dmat4 relative = glm::inverse(tform_0) * child->get_graphics_matrix() * child->collider_offset;
		glm::dmat4 final = new_tform * relative;
		child->packed_tform = to_btTransform(final);
	}

}
