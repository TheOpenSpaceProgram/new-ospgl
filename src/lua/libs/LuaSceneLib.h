#pragma once
#include "../LuaLib.h"

class LuaSceneLib : public LuaLib
{
public:
	void load_to(sol::table& table) override;
};


