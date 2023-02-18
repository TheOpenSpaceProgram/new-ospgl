#include "LuaEvents.h"

void LuaEvents::load_to(sol::table& table)
{
	table.new_usertype<LuaEventHandler>("lua_event_handler",
										"sign_out", &LuaEventHandler::sign_out
	);
}
