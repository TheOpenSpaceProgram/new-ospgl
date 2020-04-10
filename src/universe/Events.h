#pragma once
#include <variant>
#include <sol.hpp>
#include <util/defines.h>

using EventArgument = std::variant<int, double, int64_t, std::string>;

using EventArguments = std::vector<EventArgument>;

typedef void(*EventHandlerFnc)(EventArguments&, sol::function lua_fnc);

struct EventHandler
{
	EventHandlerFnc fnc;
	sol::function lua_fnc; //< Optional, only present on callbacks created from lua

	EventHandler() : fnc(nullptr), lua_fnc(sol::nil) {}

	bool operator==(const EventHandler& other) const
	{
		return fnc == other.fnc && lua_fnc == other.lua_fnc;
	}
};

struct EventHandlerHasher
{
	size_t operator()(const EventHandler& handle) const
	{
		size_t seed = 0;
		hash_combine(seed, handle.fnc);
		return seed;
	}
};
