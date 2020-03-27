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

void Vehicle::update(double dt)
{
	if(packed)
	{

	}
	else
	{
		auto n_vehicles = unpacked_veh.update();
	}	
}

Vehicle::Vehicle() : Drawable(), unpacked_veh(this), packed_veh(this)
{

}

Vehicle::~Vehicle() 
{
}
