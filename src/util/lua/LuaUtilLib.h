#pragma once
#include <sol.hpp>
#include <glm/glm.hpp>
#include "../../universe/body/config/PlanetConfig.h"

class LuaUtilLib
{
public:
	static void load_lib(sol::state& lua_state)
	{
		lua_state["make_color"] = [&lua_state](double r, double g, double b)
		{
			sol::table out_table = lua_state.create_table_with("r", r, "g", g, "b", b);
			return out_table;
		};

		lua_state["mix_color"] = [&lua_state](sol::table a, sol::table b, double t)
		{
			sol::table out_table = lua_state.create_table_with(
				"r", a["r"].get_or(0.0) * (1.0 - t) + b["r"].get_or(0.0) * t, 
				"g", a["g"].get_or(0.0) * (1.0 - t) + b["g"].get_or(0.0) * t, 
				"b", a["b"].get_or(0.0) * (1.0 - t) + b["b"].get_or(0.0) * t);

			return out_table;

		};

		lua_state["clamp"] = [](double val, double min, double max)
		{
			return glm::clamp(val, min, max);
		};
	}
};