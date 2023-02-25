#pragma once
#include "../LuaLib.h"

class LuaEditor : public LuaLib
{
public:

	virtual void load_to(sol::table& table) override;
};
