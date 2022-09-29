#pragma once
#include "../LuaLib.h"

class LuaRenderer : public LuaLib
{
public:
	void load_to(sol::table& table) override;
};



