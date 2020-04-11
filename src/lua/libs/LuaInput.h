#pragma once
#include "../LuaLib.h"

class LuaInput : public LuaLib
{
public:

	virtual void load_to(sol::table& table) override;
};
