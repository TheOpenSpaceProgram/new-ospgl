#pragma once
#include "../Entity.h"
#include "../trajectory/LandedTrajectory.h"
#include <assets/BuildingPrototype.h>

// A Building is something fixed to the ground
class BuildingEntity : public Entity
{
private:

	LandedTrajectory traj;

	btRigidBody* rigid;
	AssetHandle<BuildingPrototype> proto;

public:
	BuildingEntity(std::string in_planet);
	~BuildingEntity();

	virtual Trajectory* get_trajectory() override;
	virtual void enable_bullet(btDynamicsWorld * world) override;
	virtual void disable_bullet(btDynamicsWorld * world) override;
	virtual double get_physics_radius() override;
};

