#pragma once
#include "../Entity.h"
#include "../trajectory/LandedTrajectory.h"
// A Building is something fixed to the ground that also has a
// 3d model attached (that model will have physics).
class BuildingEntity : public Entity
{
private:

	LandedTrajectory traj;

public:
	BuildingEntity(std::string in_planet);
	~BuildingEntity();

	virtual Trajectory* get_trajectory() override;
	virtual void enable_bullet(btDynamicsWorld * world) override;
	virtual void disable_bullet(btDynamicsWorld * world) override;
	virtual double get_physics_radius() override;
};

