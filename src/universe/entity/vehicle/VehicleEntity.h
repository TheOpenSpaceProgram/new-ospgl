#pragma once
#include "../Entity.h"
#include "../../vehicle/Vehicle.h"

class VehicleEntity : public Entity
{
public:

	// This actually contains the representation of the vehicle,
	// including pieces
	Vehicle* vehicle;


	virtual Trajectory* get_trajectory() override;
	virtual void enable_bullet(btDynamicsWorld * world) override;
	virtual void disable_bullet(btDynamicsWorld * world) override;
	virtual double get_physics_radius() override;

	virtual void init();
	virtual void update(double dt);
	virtual void physics_update(double pdt);

	VehicleEntity(Vehicle* vehicle);
	~VehicleEntity();
};

