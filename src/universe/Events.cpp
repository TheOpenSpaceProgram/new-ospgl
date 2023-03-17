#include "Events.h"
#include <util/Logger.h>

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
	// We make a copy for safe iteration, as elements may be removed during iteration
	// Note that this means that removing an event handler during event call
	// will only have effect on next event!
	auto rc = index_event_receivers(event_id);

	for (auto& ev : rc)
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

