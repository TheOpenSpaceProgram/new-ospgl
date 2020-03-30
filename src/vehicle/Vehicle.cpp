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
}

std::vector<Vehicle*> Vehicle::update(double dt)
{
	for(Part* part : parts)
	{
		part->update(dt);
	}

	if(packed)
	{
		return std::vector<Vehicle*>();
	}
	else
	{
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
