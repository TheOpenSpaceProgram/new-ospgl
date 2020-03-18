#include "BuildingEntity.h"



BuildingEntity::BuildingEntity(std::string in_planet) : traj(in_planet)
{

}

BuildingEntity::~BuildingEntity()
{

}

Trajectory* BuildingEntity::get_trajectory()
{
	return &traj;
}

void BuildingEntity::enable_bullet(btDynamicsWorld* world)
{

}

void BuildingEntity::disable_bullet(btDynamicsWorld* world)
{

}

double BuildingEntity::get_physics_radius()
{
	return 0.0;
}
