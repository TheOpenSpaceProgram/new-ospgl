#pragma once 
#include "../LuaLib.h"

class LuaToml : public LuaLib 
{
public:

	virtual void load_to(sol::table& table);

};
