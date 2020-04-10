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
	vehicle->update(dt);
}

void VehicleEntity::physics_update(double pdt)
{
	auto n_vehicles = vehicle->physics_update(pdt);
	for(Vehicle* n_vehicle : n_vehicles)
	{
		get_universe()->create_entity<VehicleEntity>(n_vehicle);
	}
}

VehicleEntity::VehicleEntity(Vehicle* vehicle)
{
	this->vehicle = vehicle;
}

VehicleEntity::VehicleEntity(cpptoml::table& toml)
{

}


VehicleEntity::~VehicleEntity()
{
	delete vehicle;
}

void VehicleEntity::enable_bullet(btDynamicsWorld * world)
{
}

void VehicleEntity::disable_bullet(btDynamicsWorld * world)
{
}
