#pragma once
#include "Trajectory.h"
#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)
#include <sol/sol.hpp>

#include <renderer/Drawable.h>
#include <util/defines.h>
#include <set>

#include <cpptoml.h>

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
class Entity : public Drawable
{
private:

	Universe* universe;
	bool bullet_enabled;

	int64_t uid;

public:

	// You should start simulating bullet physics here
	virtual void enable_bullet(btDynamicsWorld* world) {}
	// You must stop simulating bullet physics here
	virtual void disable_bullet(btDynamicsWorld* world) {}

	// Return our position to be used by physics loading
	virtual glm::dvec3 get_physics_origin() { return glm::dvec3(0, 0, 0); }

	// Return position to be used as a sort of visual origin (for camera centering, etc...)
	virtual glm::dvec3 get_visual_origin() { return glm::dvec3(0, 0, 0); }

	// An approximation of our size, try to go higher than the real number
	// Values of 0.0 means that we don't have a limit for physics loading
	virtual double get_physics_radius() { return  0.0; }

	// Return true if physics are required around this entity
	// (This entity will also be loaded)
	virtual bool is_physics_loader() { return false; }

	// Visual update, always realtime
	virtual void update(double dt) {};

	// Ticks alongside bullet (bullet tick callback)
	// Note: Ticks before bullet update! (pretick)
	virtual void physics_update(double pdt) {};

	// Called when the entity is added into the universe
	// Universe is already initialized
	virtual void init() {};

	// Return true if the physics have stabilized enough for timewarp
	// Vehicles should return false when they are close enough to surfaces
	// or in atmospheric flight
	virtual bool timewarp_safe() { return true; }

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
	
	virtual std::string get_type() = 0;

	// Used while loading saves 
	static Entity* load_entity(std::string type, cpptoml::table& toml);

	virtual ~Entity();
};
