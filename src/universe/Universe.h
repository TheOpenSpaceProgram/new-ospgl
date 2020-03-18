#pragma once
#include "PlanetarySystem.h"
#include "entity/Entity.h"
#include <any>

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
// - OSPGL events are prefixed with 'osp:'
// - Mod events should be prefixed with the mod's id and ':', for example interstellar:start_hyperspace
// This will hopefully avoid event name clashing.
class Universe
{
private:

	std::unordered_map<std::string, std::set<Entity*>> event_receivers;

	std::set<Entity*>& index_event_receivers(const std::string& str);

	void sign_up_for_event(const std::string& event_id, Entity* id);
	void drop_out_of_event(const std::string& event_id, Entity* id);
	void emit_event(Entity* emitter, const std::string& event_id, VectorOfAny args = VectorOfAny());



public:

	friend class Entity;

	PlanetarySystem system;
	std::vector<Entity*> entities;

	template<typename T, typename... Args> 
	T* create_entity(Args... args);

	template<typename T>
	void remove_entity(T* ent);


	Universe();
	~Universe();
};

template<typename T, typename ...Args>
inline T* Universe::create_entity(Args... args)
{
	static_assert(std::is_base_of<Entity, T>::value, "Entities must inherit from the Entity class");

	T* n_ent = new T(args);
	Entity* as_ent = (Entity*)n_ent;
	entities.push_back((Entity*)n_ent);
	
	emit_event(nullptr, "osp:new_entity", { as_ent });

	as_ent->setup(this);

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

	emit_event(nullptr, "osp:remove_entity", { as_ent });

	// Actually destroy the entity
	delete ent;
}
