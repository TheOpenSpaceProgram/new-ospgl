#include "VehicleEntity.h"



VehicleEntity::VehicleEntity()
{
}


VehicleEntity::~VehicleEntity()
{
}

Trajectory * VehicleEntity::get_trajectory()
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
