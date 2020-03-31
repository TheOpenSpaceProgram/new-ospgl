#include "Vehicle.h"

void Vehicle::deferred_pass(CameraUniforms & camera_uniforms)
{
	for (Piece* p : all_pieces)
	{
		glm::dmat4 tform = glm::inverse(p->collider_offset) * to_dmat4(p->get_global_transform());
		p->model_node->draw(camera_uniforms, tform, true);
	}
}

void Vehicle::unpack()
{
	logger->check_important(packed, "Tried to unpack an unpacked vehicle");
	
	packed = false;
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
		// glm::dvec3 pos = unpacked_veh.get_center_of_mass(); 	//< Not neccesary, in-vehicle distances are minimal
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

Vehicle::Vehicle() : Drawable(), unpacked_veh(this), packed_veh(this)
{
}

Vehicle::~Vehicle() 
{
}
