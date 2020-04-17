#pragma once 
#include "../LuaLib.h"

class LuaGameDatabase : public LuaLib
{
public:

	virtual void load_to(sol::table& table) override;

};
