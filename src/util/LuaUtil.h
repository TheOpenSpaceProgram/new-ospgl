#pragma once
#include <lua/LuaCore.h>
#include "Logger.h"


class LuaUtil
{
public:

	static void safe_lua(sol::state& state, const std::string& script, bool& wrote_error, const std::string& script_path)
	{
		state.safe_script(script, [&wrote_error, &script_path](lua_State*, sol::protected_function_result pfr)
		{

			if (!wrote_error)
			{
				sol::error err = pfr;
				logger->error("Lua Error ({}):\n{}", script_path, err.what());
				wrote_error = true;
			}

			return pfr;
		}, script_path);
	}

	// Handles errors
	template<typename T, typename... Args>
	static sol::safe_function_result call_function(T& env,
			const std::string& name, const std::string& context, Args&&... args)
	{
		sol::safe_function fnc = env[name];

		auto result = fnc(std::forward<Args>(args)...);

		if(!result.valid())
		{
			sol::error as_error = result;
			logger->error("Lua Error in {}:\n{}", context, as_error.what());
		}	

		return result;	
	}

	// Crashes on error
	template<typename T, typename... Args>
	static sol::safe_function_result safe_call_function(T& env, 
			const std::string& name, const std::string& context, Args&&... args)
	{
		sol::safe_function fnc = env[name];
		auto result = fnc(std::forward<Args>(args)...);

		if(!result.valid())
		{
			sol::error as_error = result;
			logger->fatal("Lua Error in {}:\n{}", context, as_error.what());
		}	

		return result;	
	}

	// Same as call_function but only does it if function is present
	template<typename T, typename... Args>
	static sol::safe_function_result call_function_if_present(T& env,
		const std::string& name, const std::string& context, Args&&... args)
	{
		sol::safe_function fnc = env[name];
		if(fnc)
		{
			return call_function(env, name, context, args...);
		}
		else
		{
			// This makes the error clearer, but may cause a perfomance hit
			return fnc(std::forward<Args>(args)...);
		}
	}
};
