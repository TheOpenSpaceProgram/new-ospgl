#include "LuaLogger.h"

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

	return fmt::format("{} ({})", file, line);
}

void LuaLogger::load_to(sol::table& table)
{
	table["debug"] = [](sol::this_state s, std::string str)
	{
		logger->debug("[{}]: {}", get_debug_trace(s.L), str);
	};

	table["info"] = [](sol::this_state s, std::string str)
	{
		logger->info("[{}]: {}", get_debug_trace(s.L), str);
	};

	table["warn"] = [](sol::this_state s, std::string str)
	{
		logger->warn("[{}]: {}", get_debug_trace(s.L), str);
	};

	table["error"] = [](sol::this_state s, std::string str)
	{
		logger->error("[{}]: {}", get_debug_trace(s.L), str);
	};

	table["fatal"] = [](sol::this_state s, std::string str)
	{
		logger->fatal("[{}]: {}", get_debug_trace(s.L), str);
	};
}
