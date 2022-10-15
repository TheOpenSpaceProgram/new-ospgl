#pragma once
#include "../Entity.h"
#include "../../vehicle/Vehicle.h"
#include "VehicleDebug.h"

// We take ownership of the vehicle pointer (we will delete it)
// so make sure it's heap allocated
// TODO: This could also be implemented in lua, but I've chosen against it
// as there's not really any reason to have user code here
class VehicleEntity : public Entity
{
public:

	// This actually contains the representation of the vehicle,
	// including pieces
	Vehicle* vehicle;
	VehicleDebug debug;


	void enable_bullet(btDynamicsWorld * world) override;
	void disable_bullet(btDynamicsWorld * world) override;

	void init() override;
	void update(double dt) override;
	void physics_update(double pdt) override;

	void deferred_pass(CameraUniforms& camera_uniforms, bool is_env) override;
	bool needs_deferred_pass() override { return true; }

	void shadow_pass(ShadowCamera& sh_camera) override;
	bool needs_shadow_pass() override { return true; }

	glm::dvec3 get_physics_origin() override;
	glm::dvec3 get_visual_origin() override;

	VehicleEntity(Vehicle* vehicle);
	VehicleEntity(cpptoml::table& toml);
	~VehicleEntity();

	virtual std::string get_type() override { return "vehicle"; }
};

