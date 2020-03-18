#pragma once
#include "Trajectory.h"
#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)

#include "../../renderer/Drawable.h"
#include "../../util/defines.h"
#include <set>

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

	std::set<std::string> signed_up_events;

public:

	virtual Trajectory* get_trajectory() = 0;
	virtual void enable_bullet(btDynamicsWorld* world) = 0;
	virtual void disable_bullet(btDynamicsWorld* world) = 0;

	// An approximation of our size, try to go higher than the real number
	virtual double get_physics_radius() = 0;

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

	void setup(Universe* universe)
	{
		this->universe = universe;

		init();
	}

	Universe* get_universe()
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

	virtual void receive_event(Entity* emitter, const std::string& event_id, VectorOfAny args = VectorOfAny()) {}

	// Note: You don't need to manually drop out of events before destruction, the destructor will do it for you
	void emit_event(const std::string& event_id, VectorOfAny args = VectorOfAny());
	void sign_up_for_event(const std::string& event_id);
	void drop_out_of_event(const std::string& event_id);

	~Entity();
};