#pragma once
#include "../LuaLib.h"

class LuaDebugDrawer : public LuaLib
{
public:

	virtual void load_to(sol::table& table);

};