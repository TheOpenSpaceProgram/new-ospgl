#pragma once
#include <variant>
#include <sol/sol.hpp>
#include <util/defines.h>
#include <unordered_set>

using EventArgument = std::variant<int, double, int64_t, std::string>;

using EventArguments = std::vector<EventArgument>;

typedef void(*EventHandlerFncPtr)(EventArguments&);
using EventHandlerFnc = std::function<void(EventArguments& args)>;

struct EventHandler
{
	EventHandlerFnc fnc;

	EventHandler() : fnc(nullptr) {}

	bool operator==(const EventHandler& other) const
	{
		return fnc.target<EventHandlerFncPtr>() == other.fnc.target<EventHandlerFncPtr>();
	}

	EventHandler(EventHandlerFnc nf) : fnc(nf) {}
};

struct EventHandlerHasher
{
	size_t operator()(const EventHandler& handle) const
	{
		size_t seed = 0;
		hash_combine(seed, handle.fnc.target<EventHandlerFncPtr>());
		return seed;
	}
};

class EventEmitter
{
private:
	std::unordered_map<std::string, std::unordered_set<EventHandler, EventHandlerHasher>> event_receivers;
	std::unordered_set<EventHandler, EventHandlerHasher>& index_event_receivers(const std::string& str);

public:

	void sign_up_for_event(const std::string& event_id, EventHandler id);
	void drop_out_of_event(const std::string& event_id, EventHandler id);
	void emit_event(const std::string& event_id, EventArguments args = EventArguments());
	template<typename... Args>
	void emit_event(const std::string& event_id, Args&&... args)
	{
		EventArguments vc {args...};
		emit_event(event_id, vc);
	}


};
