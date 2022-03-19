#pragma once
#include "../LuaLib.h"

class LuaImGUI : public LuaLib
{
public:

	virtual void load_to(sol::table& table) override;
};
