#pragma once
#include "Trajectory.h"
#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)
#include <sol/sol.hpp>

#include <renderer/Drawable.h>
#include <util/defines.h>
#include <set>

#include "history/EntityHistory.h"

#include <cpptoml.h>

class InputContext;

// An entity is something which exists on the world, 
// it has graphics, and can exists on the bullet physics
// world.
// Planets are not entities for perfomance, but they could
// be (TODO: Maybe it's a good idea?)
//
// Physics are enabled on a variety of conditions:
// Note that focusing does not necessarily mean actually being playing as said entity
// it can be enabled for any entity by the user (for example Falcon 9 boosters)
// - While timewarp is low:
//		-> If the entity is focused
//		-> If the entity is near a planet surface and it is not landed
//		-> If the entity is close enough to a focused entity
//
// Physics can't be enabled while timewarp is high, but timewarp will warn the user
// if any entity cannot currently timewarp because of physics (it can optionally
// block or reduce timewarp)
// We store history for entities. History is generated automaticaly, and configured
// by the entity itself.
// Predictions, as they are computationally expensive, are generated on demand externally to the entity
// which may optionally not have n-body behaviour with the has_custom_propagation flag
class Entity : public Drawable
{
private:

	Universe* universe;
	bool bullet_enabled;
	int64_t uid;
	sol::state* lua_state;
	std::string type_str;

public:
	EntityHistory history;

	sol::environment env;
	std::shared_ptr<cpptoml::table> init_toml;

	// You should start simulating bullet physics here
	void enable_bullet(btDynamicsWorld* world);
	// You must stop simulating bullet physics here
	void disable_bullet(btDynamicsWorld* world);

	// Give your current position, velocity and orientation
	// Origin may be a representative point, for example, COM
	glm::dvec3 get_position();
	glm::dvec3 get_velocity();
	glm::dquat get_orientation();
	glm::dvec3 get_angular_velocity();

	// If you return nullptr, it will be assumed you are n-body propagated
	// Otherwise, return a valid Trajectory for the predictor / propagator
	// Note that you need to manually call update() in the trajectory! This is only
	// for timewarp and prediction
	Trajectory* get_trajectory();

	// An approximation of our size, try to go higher than the real number
	// Values of 0.0 means that we don't have a limit for physics loading
	double get_physics_radius();

	// Return true if physics are required around this entity
	// (This entity will also be loaded)
	bool is_physics_loader();

	// Visual update, always realtime
	void update(double dt);

	// If it returns nullptr, then you don't have a input_ctx
	InputContext* get_input_ctx();

	// Ticks alongside bullet (bullet tick callback)
	// Note: Ticks before bullet update! (pretick)
	void physics_update(double pdt);

	// Called when the entity is added into the universe
	// Universe is already initialized
	void init();

	// Return true if the physics have stabilized enough for timewarp
	// Vehicles should return false when they are close enough to surfaces
	// or in atmospheric flight
	bool timewarp_safe();

	void setup(Universe* universe, int64_t uid);

	inline int64_t get_uid()
	{
		return uid;
	}

	inline Universe* get_universe()
	{
		return universe;
	}

	void enable_bullet_wrapper(bool value, btDynamicsWorld* world)
	{
		bullet_enabled = value;

		if (value)
		{
			enable_bullet(world);
		}
		else
		{
			disable_bullet(world);
		}
	}

	std::string get_type();

	// Used while loading saves
	static Entity* load(std::string type, std::shared_ptr<cpptoml::table> toml);
	void save(cpptoml::table& to);

	explicit Entity(std::string script_path, std::string in_pkg, std::shared_ptr<cpptoml::table> init_toml,
	std::vector<sol::object> args, bool is_create);

	void deferred_pass(CameraUniforms& cu, bool is_env_map = false) override;
	void forward_pass(CameraUniforms& cu, bool is_env_map = false) override;
	void gui_pass(CameraUniforms& cu) override;
	void shadow_pass(ShadowCamera& cu) override;
	void far_shadow_pass(ShadowCamera& cu) override;
	bool needs_deferred_pass() override;
	bool needs_forward_pass() override;
	bool needs_gui_pass() override;
	bool needs_shadow_pass() override;
	bool needs_far_shadow_pass() override;
	bool needs_env_map_pass() override;

	// This is called already within a window so don't create one
	void do_debug_imgui();


	// Objects with higher priority get drawn first
	virtual int get_forward_priority() { return 0.0; }

	~Entity();

};
