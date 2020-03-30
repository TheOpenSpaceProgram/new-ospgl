#include "VehicleEntity.h"

#include "../../../renderer/Renderer.h"
#include "../../Universe.h"

void VehicleEntity::init()
{
	get_universe()->renderer->add_drawable(this->vehicle);
	this->vehicle->set_world(get_universe()->bt_world);
	this->vehicle->init(get_universe());
}

void VehicleEntity::update(double dt)
{
	auto n_vehicles = vehicle->update(dt);
	for(Vehicle* n_vehicle : n_vehicles)
	{
		get_universe()->create_entity<VehicleEntity>(n_vehicle);
	}
}

void VehicleEntity::physics_update(double pdt)
{

}

VehicleEntity::VehicleEntity(Vehicle* vehicle)
{
	this->vehicle = vehicle;
}


VehicleEntity::~VehicleEntity()
{
}

Trajectory* VehicleEntity::get_trajectory()
{
	return nullptr;
}

void VehicleEntity::enable_bullet(btDynamicsWorld * world)
{
}

void VehicleEntity::disable_bullet(btDynamicsWorld * world)
{
}

double VehicleEntity::get_physics_radius()
{
	return 0.0;
}
