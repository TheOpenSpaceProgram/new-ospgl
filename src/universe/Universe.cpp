#include "Universe.h"



std::set<Entity*>& Universe::index_event_receivers(const std::string & str)
{
	auto it = event_receivers.find(str);
	if (it == event_receivers.end())
	{
		event_receivers[str] = std::set<Entity*>();
		return event_receivers[str];
	}
	else
	{
		return it->second;
	}
}

void Universe::emit_event(Entity* emitter, const std::string& event_id, VectorOfAny args)
{
	std::set<Entity*>& rc = index_event_receivers(event_id);

	for (Entity* e : rc)
	{
		e->receive_event(emitter, event_id, args);
	}
}

void Universe::sign_up_for_event(const std::string& event_id, Entity* id)
{
	std::set<Entity*>& rc = index_event_receivers(event_id);

	rc.insert(id);
}

void Universe::drop_out_of_event(const std::string& event_id, Entity* id)
{
	std::set<Entity*>& rc = index_event_receivers(event_id);

	rc.erase(id);
}

Universe::Universe()
{
}


Universe::~Universe()
{
}
