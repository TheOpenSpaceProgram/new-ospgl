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

	// Handles errors
	template<typename... Args>
	static sol::safe_function_result call_function(sol::state& st, 
			const std::string& fname, const std::string& context, Args&&... args)
	{
		sol::safe_function fnc = st[fname];
		
		auto result = fnc(std::forward<Args>(args)...);

		if(!result.valid())
		{
			sol::error as_error = result;
			logger->error("Lua Error in {}:\n{}", context, as_error.what());
		}	

		return result;	
	}

	// Crashes on error
	template<typename... Args>
	static sol::safe_function_result safe_call_function(sol::state& st, 
			const std::string& fname, const std::string& context, Args&&... args)
	{
		sol::safe_function fnc = st[fname];
		
		auto result = fnc(std::forward<Args>(args)...);

		if(!result.valid())
		{
			sol::error as_error = result;
			logger->fatal("Lua Error in {}:\n{}", context, as_error.what());
		}	

		return result;	
	}
};
