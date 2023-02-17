#pragma once
#include "../LuaLib.h"
#include <universe/Universe.h>


// 
// Events:
//
// You can sign up for an event using universe.sign_up(event_id),
// the return variable allows you to unsubscribe from the event, and 
// will do so automatically on deletion (cast to nil or gargabe collection)
//
// This also includes the planetary system
// (TODO: Maybe do it separately?)
class LuaUniverse : public LuaLib
{
public:

	virtual void load_to(sol::table& table) override;
};
