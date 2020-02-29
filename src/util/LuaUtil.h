#pragma once
#include "../lua/LuaCore.h"
#include "Logger.h"

class LuaUtil
{
public:

	static void safe_lua(sol::state& state, const std::string& script, bool& wrote_error, const std::string& script_path)
	{
		state.safe_script(script, [&wrote_error](lua_State*, sol::protected_function_result pfr)
		{

			if (!wrote_error)
			{
				sol::error err = pfr;
				logger->error("Lua Error:\n{}", err.what());
				wrote_error = true;
			}

			return pfr;
		}, script_path);
	}

};