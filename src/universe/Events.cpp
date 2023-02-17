#include "Events.h"

std::unordered_set<EventHandler, EventHandlerHasher>& EventEmitter::index_event_receivers(const std::string& str)
{
	auto it = event_receivers.find(str);
	if (it == event_receivers.end())
	{
		event_receivers[str] = std::unordered_set<EventHandler, EventHandlerHasher>();
		return event_receivers[str];
	}
	else
	{
		return it->second;
	}
}

void EventEmitter::emit_event(const std::string& event_id, EventArguments args)
{
	auto& rc = index_event_receivers(event_id);

	for (EventHandler ev : rc)
	{
		ev.fnc(args);
	}
}


void EventEmitter::sign_up_for_event(const std::string& event_id, EventHandler id)
{
	auto& rc = index_event_receivers(event_id);
	rc.insert(id);
}


void EventEmitter::drop_out_of_event(const std::string& event_id, EventHandler id)
{
	auto& rc = index_event_receivers(event_id);
	rc.erase(id);
}

