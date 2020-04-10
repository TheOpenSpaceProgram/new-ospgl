#pragma once
#include "../Entity.h"
#include "../../vehicle/Vehicle.h"

// We take ownership of the vehicle pointer (we will delete it)
// so make sure it's heap allocated
class VehicleEntity : public Entity
{
public:

	// This actually contains the representation of the vehicle,
	// including pieces
	Vehicle* vehicle;


	virtual void enable_bullet(btDynamicsWorld * world) override;
	virtual void disable_bullet(btDynamicsWorld * world) override;

	virtual void init() override;
	virtual void update(double dt) override;
	virtual void physics_update(double pdt) override;

	VehicleEntity(Vehicle* vehicle);
	VehicleEntity(cpptoml::table& toml);
	~VehicleEntity();

	virtual std::string get_type() override { return "vehicle"; }
};

