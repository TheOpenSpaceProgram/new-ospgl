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


	void enable_bullet(btDynamicsWorld * world) override;
	void disable_bullet(btDynamicsWorld * world) override;

	void init() override;
	void update(double dt) override;
	void physics_update(double pdt) override;

	void deferred_pass(CameraUniforms& camera_uniforms, bool is_env) override;
	bool needs_deferred_pass() override { return true; }

	void shadow_pass(ShadowCamera& sh_camera) override;
	bool needs_shadow_pass() override { return true; }

	VehicleEntity(Vehicle* vehicle);
	VehicleEntity(cpptoml::table& toml);
	~VehicleEntity();

	virtual std::string get_type() override { return "vehicle"; }
};

