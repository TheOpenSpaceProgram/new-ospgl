#pragma once
#include "PlanetarySystem.h"
#include "entity/Entity.h"
#include <any>
#include <unordered_set>
#include "Events.h"
#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.h>
#pragma warning(pop)

#include <physics/debug/BulletDebugDrawer.h>
#include <sol/sol.hpp>

// The Universe is the central class of the game. It stores both the system
// and everything else in the system (buildings and vehicles).
// It implements a little Entity system to simplify a lot of the code
// and make modder's life easier. 
// Note that events are implemented fully dynamic as they are needed
// from the lua side. Otherwise we could simply use a events library.
//
// Events can carry any set of arguments, handled as a std::vector<std::any>
// It's up to the event how are these arguments handled.
// Global events have "emitter" set to nullptr
// Event naming:
// - OSPGL events are prefixed with 'core:'
// - Mod events should be prefixed with the mod's id and ':', for example interstellar:start_hyperspace
// This will hopefully avoid event name clashing.
// 
// It's the responsability of the event receiver to remove the handler once it's deleted / not needed!
class GameState;

class Universe
{
private:

	std::unordered_map<std::string, std::unordered_set<EventHandler, EventHandlerHasher>> event_receivers;

	std::unordered_set<EventHandler, EventHandlerHasher>& index_event_receivers(const std::string& str);


	btDefaultCollisionConfiguration* bt_collision_config;
	btCollisionDispatcher* bt_dispatcher;
	btBroadphaseInterface* bt_brf_interface;
	btSequentialImpulseConstraintSolver* bt_solver;

	BulletDebugDrawer* bt_debug;

	int64_t uid;

public:

	// Should updates run?
	bool paused;

	friend class Entity;
	friend class GameState;

	static constexpr double PHYSICS_STEPSIZE = 1.0 / 30.0;
	static constexpr int MAX_PHYSICS_STEPS = 1;

	// We use a global state for everything in the universe so data can 
	// be shared between lua scripts without "hacks"
	// Planet surfaces are independent, and other stuff, as they are not sharing
	// any data with the rest of the universe
	sol::state lua_state;

#ifdef OSPGL_LRDB
	void* lua_debug_server;
	// This function will block until the debugger is connected!
	void enable_debugging();
	void disable_debugging();
#endif

	void sign_up_for_event(const std::string& event_id, EventHandler id);
	void drop_out_of_event(const std::string& event_id, EventHandler id);
	void emit_event(const std::string& event_id, EventArguments args = EventArguments());
	template<typename... Args>
	void emit_event(const std::string& event_id, Args&&... args)
	{
		EventArguments vc {args...};
		emit_event(event_id, vc);	
	}


	btDiscreteDynamicsWorld* bt_world;

	PlanetarySystem system;
	std::vector<Entity*> entities;
	std::unordered_map<int64_t, Entity*> entities_by_id;

	template<typename T, typename... Args>
	T* create_entity(Args&&... args);

	template<typename T>
	void remove_entity(T* ent);

	// Returns nullptr if not found
	Entity* get_entity(int64_t id);

	template<typename T> 
	T* get_entity_as(int64_t id);

	// Note: This is automatically called from bullet
	void physics_update(double pdt);
	void update(double dt);
	
	int64_t get_uid();

	Universe();
	~Universe();
};

template<typename T, typename ...Args>
inline T* Universe::create_entity(Args&&... args)
{
	static_assert(std::is_base_of<Entity, T>::value, "Entities must inherit from the Entity class");

	T* n_ent = new T(std::forward<Args>(args)...);
	Entity* as_ent = (Entity*)n_ent;

	int64_t id = get_uid();

	entities.push_back((Entity*)n_ent);
	entities_by_id[id] =  as_ent;

	emit_event("core:new_entity", id);
	
	as_ent->setup(this, id);

	return n_ent;
}

template<typename T>
inline void Universe::remove_entity(T* ent)
{
	static_assert(std::is_base_of<Entity, T>::value, "Entities must inherit from the Entity class");

	Entity* as_ent = (Entity*)ent;
	for (auto it = entities.begin(); it != entities.end(); it++)
	{
		if (*it == ent)
		{
			entities.erase(it);
			break;
		}
	}

	emit_event("core:remove_entity", as_ent->get_uid());

	// Remove from entities by id
	entities_by_id.erase(as_ent->get_uid());
	
	// Actually destroy the entity
	delete ent;
}

template<typename T>
inline T* Universe::get_entity_as(int64_t id)
{
	static_assert(std::is_base_of<Entity, T>::value, "Entities must inherit from the Entity class");
	Entity* ent = get_entity(id);
	return dynamic_cast<T*>(ent);
}

