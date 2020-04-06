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

	glm::dmat4 get_model_matrix(bool bullet);

public:

	BuildingEntity(AssetHandle<BuildingPrototype>&& proto);
	BuildingEntity(cpptoml::table& toml);

	~BuildingEntity();

	virtual void enable_bullet(btDynamicsWorld* world) override;
	virtual void disable_bullet(btDynamicsWorld* world) override;

	virtual void init() override;

	virtual void physics_update(double pdt) override;

	virtual std::string get_type() override { return "building"; }

	virtual void deferred_pass(CameraUniforms& cu) override;
	virtual void shadow_pass(ShadowCamera& cu) override;

	virtual bool needs_deferred_pass() { return true; }
	virtual bool needs_shadow_pass() { return true; }
};

