#include "Vehicle.h"

void Vehicle::unpack()
{
	logger->check_important(packed, "Tried to unpack an unpacked vehicle");
	
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
	logger->check_important(!packed, "Tried to pack a packed vehicle");

	packed = true;

	unpacked_veh.deactivate();

	packed_veh.calculate_com();
}

void Vehicle::update(double dt)
{
	for(Part* part : parts)
	{
		part->update(dt);
	}

	// Clear blocked ports
	for (Port* p : all_ports)
	{
		p->blocked = false;
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
	for(Part* part : parts)
	{
		part->init(in_universe, this);
	}

	this->in_universe = in_universe;

	// Init wires
	if(wires_init)
	{
		// Load wires
		for(auto wire : *wires_init)
		{
			int64_t from_id = *wire->get_as<int64_t>("from");
			int64_t to_id = *wire->get_as<int64_t>("to");
			std::string from_machine = *wire->get_as<std::string>("fmachine");
			std::string to_machine = *wire->get_as<std::string>("tmachine");
			std::string from_port = *wire->get_as<std::string>("fport");
			std::string to_port = *wire->get_as<std::string>("tport");

			Part* from = get_part(from_id);
			Part* to = get_part(to_id);

			Machine* from_mach = from->get_machine(from_machine);
			Machine* to_mach = to->get_machine(to_machine);

			Port* from_prt = from_mach->get_output_port(from_port);
			Port* to_prt = to_mach->get_input_port(to_port);
			
			from_prt->to.push_back(to_prt);
		}

		wires_init = nullptr;

		check_wires();
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

void Vehicle::check_wires()
{
	std::unordered_set<Port*> seen_targets;
	for(auto port_it = all_ports.begin(); port_it != all_ports.end(); )
	{
		bool bad = false;
		Port* port = *port_it;

		if (port->is_output)
		{
			for (Port* target : port->to)
			{
				if (target->is_output)
				{
					logger->error("Vehicle has wire with output set to an output port");
					bad = true;
				}

				if (target->type != port->type)
				{
					logger->error("Vehicle has wire with mismatching types");
					bad = true;
				}

				if (seen_targets.find(target) != seen_targets.end())
				{
					logger->error("A port has multiple inputs");
					bad = true;
				}

				seen_targets.insert(target);
			}
		}
		else
		{
			if(!port->to.empty())
			{ 
				logger->error("Input port has ports set to its output");
				bad = true;
			}
		}

		if(bad)
		{
			logger->fatal("Malformed vehicle wires");
			return;
		}
		else
		{
			port_it++;
		}
	}
}

Piece* Vehicle::get_piece(int64_t id)
{
	auto it = id_to_piece.find(id);
	logger->check_important(it != id_to_piece.end(), "Wrong piece id");
	return it->second;
}

Part* Vehicle::get_part(int64_t id)
{
	auto it = id_to_part.find(id);
	logger->check_important(it != id_to_part.end(), "Wrong piece id");
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
