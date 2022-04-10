#include "LuaLogger.h"
#include <universe/vehicle/part/Machine.h>
#include <universe/vehicle/part/Part.h>

static std::string get_debug_trace(lua_State* L)
{
	lua_Debug ar;
	lua_getstack(L, 1, &ar);
	lua_getinfo(L, "nSl", &ar);
	int line = ar.currentline;	
	std::string file = ar.source;

	// Trim useless stuff before the actual filename
	if (file[0] == '@')
	{
		file = file.substr(1);
	}

	if (file[0] == '.')
	{
		file = file.substr(1);
	}

	if (file[0] == '/')
	{
		file = file.substr(1);
	}

	return fmt::format("{}({})", file, line);
}

// Sometimes we can give more clues to the location of the error
static std::string extra_debug(sol::environment l)
{
	if(l["machine"].valid())
	{
		Machine* m = l["machine"];
		if(vector_contains(m->in_part->attached_machines, m))
		{
			return fmt::format("\n      Attached Machine rid: {}, Part id: {} in prototype: {}", m->runtime_uid, m->in_part->id,
							   m->in_part->part_proto->get_asset_id());
		}
		else
		{
			return fmt::format("\n      Machine rid: {}, Part id: {} in prototype: {}", m->runtime_uid, m->in_part->id,
							   m->in_part->part_proto->get_asset_id());
		}
	}

	return "";
}

void LuaLogger::load_to(sol::table& table)
{
	table.set_function("debug", [](sol::this_state s, sol::this_environment e, sol::object str)
	{
		logger->debug("[{}]: {}{}", get_debug_trace(s.L),
					  sol::state_view(s).get<sol::function>("tostring")(str).get<std::string>(), extra_debug(e));
	});

	table.set_function("info", [](sol::this_state s, sol::this_environment e, sol::object str)
	{
		logger->info("[{}]: {}{}", get_debug_trace(s.L),
					 sol::state_view(s).get<sol::function>("tostring")(str).get<std::string>(), extra_debug(e));
	});

	table.set_function("warn", [](sol::this_state s, sol::this_environment e, sol::object str)
	{
		logger->warn("[{}]: {}{}", get_debug_trace(s.L),
					 sol::state_view(s).get<sol::function>("tostring")(str).get<std::string>(), extra_debug(e));
	});

	table.set_function("error", [](sol::this_state s, sol::this_environment e, sol::object str)
	{
		logger->error("[{}]: {}{}", get_debug_trace(s.L),
					  sol::state_view(s).get<sol::function>("tostring")(str).get<std::string>(), extra_debug(e));
	});

	table.set_function("fatal", [](sol::this_state s, sol::this_environment e, sol::object str)
	{
		logger->fatal("[{}]: {}{}", get_debug_trace(s.L),
					  sol::state_view(s).get<sol::function>("tostring")(str).get<std::string>(), extra_debug(e));
	});
}
