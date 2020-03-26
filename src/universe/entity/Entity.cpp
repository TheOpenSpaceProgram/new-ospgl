#include "Entity.h"
#include "../Universe.h"

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

