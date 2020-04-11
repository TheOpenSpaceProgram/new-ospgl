#include "LuaUniverse.h"

void LuaUniverse::load_to(sol::table& table)
{
	table.new_usertype<LuaEventHandler>("lua_event_handler",
		"sign_out", &LuaEventHandler::sign_out
	);

	table.new_usertype<Universe>("universe",
		"sign_up_for_event", [](Universe* self, const std::string& event_id, sol::function* fnc)
		{
			LuaEventHandler ev = LuaEventHandler();

			ev.event_id = event_id;

			EventHandlerFnc wrapper = [](EventArguments& vec, void* udata)
			{
				// Convert to lua values
				sol::function& fnc = *(sol::function*)udata;
				fnc(sol::as_args(vec));
			};

			ev.handler = EventHandler();
			ev.handler.fnc = wrapper;
			ev.handler.user_data = (void*)fnc;
			ev.universe = self;
			ev.signed_up = true;

			self->sign_up_for_event(event_id, ev.handler);

			logger->info("Signed up for event");

			return std::move(ev);
		},
		"emit_event", [](Universe* self, const std::string& event_id, sol::variadic_args va)
		{
			EventArguments any_vec = EventArguments();
			for(auto v : va)
			{
				any_vec.push_back(EventArgument(v));
			}

			self->emit_event(event_id, any_vec);	
		}	
	);
}
