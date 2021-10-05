#pragma once
#include "../LuaLib.h"
#include <util/Logger.h>

/*
	Lua functions to handle and sample images

*/
class LuaGUI : public LuaLib
{
public:
	virtual void load_to(sol::table& table) override;
	LuaGUI();
	~LuaGUI();
};



