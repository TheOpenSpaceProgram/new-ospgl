#include "Entity.h"
#include "../Universe.h"
#include "entities/VehicleEntity.h"
#include "entities/BuildingEntity.h"

void Entity::emit_event(const std::string & event_id, VectorOfAny args)
{
	universe->emit_event(this, event_id, args);
}

void Entity::sign_up_for_event(const std::string& event_id)
{
	signed_up_events.insert(event_id);
	universe->sign_up_for_event(event_id, this);
}

void Entity::drop_out_of_event(const std::string& event_id)
{
	signed_up_events.erase(event_id);
	universe->drop_out_of_event(event_id, this);
}

void Entity::setup(Universe* universe)
{
	this->universe = universe;
	this->uid = universe->get_uid();
	init();
}

Entity::~Entity()
{
	for (const std::string& ev : signed_up_events)
	{
		universe->drop_out_of_event(ev, this);
	}
}

Entity* Entity::load_entity(int64_t uid, std::string type, cpptoml::table& toml)
{
	Entity* n_ent = nullptr;

	if (type == "vehicle")
	{
		n_ent = new VehicleEntity(toml);
	}
	else if (type == "building")
	{
		n_ent = new BuildingEntity(toml);
	}
	else
	{
		logger->fatal("Unknown entity type '{}'", type);
	}

	return n_ent;
}
