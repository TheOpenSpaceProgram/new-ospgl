#pragma once
#include <lua/LuaCore.h>
#include "Logger.h"


class LuaUtil
{
public:

	static void lua_error_handler(lua_State* L, sol::error err)
	{
		std::string what_str = err.what();
		bool has_traceback = what_str.find("stack traceback:") != std::string::npos;
		if(has_traceback)
		{
			logger->error("Lua error:\n{}", what_str);
		}
		else
		{
			std::string traceback;
			int top = 1;
			for (int i = 0; i < top; i++) {
				luaL_where(L, i);
				std::string st = sol::stack::pop<std::string>(L);
				traceback += st;
				if (i != top) {
					traceback += '\n';
				}
			}

			logger->error("Lua error:\n{}. Traceback:\n{}", what_str, traceback);
		}
	}

	static void safe_lua(sol::state& state, const std::string& script, bool& wrote_error, const std::string& script_path)
	{
		state.safe_script(script, [&wrote_error, &script_path](lua_State* L, sol::protected_function_result pfr)
		{
			sol::default_traceback_error_handler(L);
			std::string traceback = sol::stack::pop<std::string>(L);

			if (!wrote_error)
			{
				sol::error err = pfr;
				lua_error_handler(L, err);
				wrote_error = true;
			}

			return pfr;
		}, script_path);
	}

	// Handles errors
	template<typename T, typename K, typename... Args>
	static sol::safe_function_result call_function(sol::table_proxy<T, K> path, const std::string& context, Args&&... args)
    {
		sol::safe_function fnc = path;


		auto result = fnc(std::forward<Args>(args)...);

		if(!result.valid())
		{
			sol::error err = result;
			lua_error_handler(path.lua_state(), err);
		}

		return result;	
	}

	// Crashes on error
	template<typename T, typename K, typename... Args>
	static sol::safe_function_result safe_call_function(sol::table_proxy<T, K> path,
			const std::string& context, Args&&... args)
	{
		sol::safe_function fnc = path;
		auto result = fnc(std::forward<Args>(args)...);

		if(!result.valid())
		{
			sol::error err = result;
			lua_error_handler(path.lua_state(), err);
		}

		return result;	
	}

	// Same as call_function but only does it if function is present
	template<typename T, typename K, typename... Args>
	static std::optional<sol::safe_function_result> 
		call_function_if_present(sol::table_proxy<T, K> path, const std::string& context, Args&&... args)
	{
		if(path.valid())
		{
			return call_function(path, context, args...);
		}
		else
		{
			return {};
		}
	}
};
