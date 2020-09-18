#pragma once
#include <sol/sol.hpp>

class LuaLib
{
public:

	virtual void load_to(sol::table& table) = 0;
};