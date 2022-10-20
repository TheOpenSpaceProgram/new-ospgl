#pragma once
#include "../LuaLib.h"

class LuaModel : public LuaLib
{
public:

	void load_to(sol::table& table) override;

};
