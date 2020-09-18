#pragma once
#include <variant>
#include <sol/sol.hpp>
#include <util/defines.h>

using EventArgument = std::variant<int, double, int64_t, std::string>;

using EventArguments = std::vector<EventArgument>;

typedef void(*EventHandlerFnc)(EventArguments&, const void* user_data);

struct EventHandler
{
	EventHandlerFnc fnc;
	const void* user_data; 

	EventHandler() : fnc(nullptr), user_data(nullptr) {}

	bool operator==(const EventHandler& other) const
	{
		return fnc == other.fnc && user_data == other.user_data;
	}

	EventHandler(EventHandlerFnc nf, void* ud) : fnc(nf), user_data(ud) {}
};

struct EventHandlerHasher
{
	size_t operator()(const EventHandler& handle) const
	{
		size_t seed = 0;
		hash_combine(seed, handle.fnc);
		hash_combine(seed, handle.user_data);
		return seed;
	}
};
