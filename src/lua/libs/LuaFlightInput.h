#pragma once
#include "../LuaLib.h"

class LuaFlightInput : public LuaLib
{
public:

	virtual void load_to(sol::table& table) override;
};
