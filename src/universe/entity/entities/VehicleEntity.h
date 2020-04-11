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

	virtual void deferred_pass(CameraUniforms& camera_uniforms) override;
	virtual bool needs_deferred_pass() override { return true; }

	virtual void shadow_pass(ShadowCamera& sh_camera) override;
	virtual bool needs_shadow_pass() override { return true; }

	VehicleEntity(Vehicle* vehicle);
	VehicleEntity(cpptoml::table& toml);
	~VehicleEntity();

	virtual std::string get_type() override { return "vehicle"; }
};

