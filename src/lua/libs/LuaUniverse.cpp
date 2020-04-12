#include "LuaUniverse.h"

void LuaUniverse::load_to(sol::table& table)
{
	table.new_usertype<LuaEventHandler>("lua_event_handler",
		"sign_out", &LuaEventHandler::sign_out
	);

	table.new_usertype<Universe>("universe",
		"sign_up_for_event", [](Universe* self, const std::string& event_id, sol::function fnc)
		{
			LuaEventHandler ev = LuaEventHandler();

			ev.event_id = event_id;

			EventHandlerFnc wrapper = [](EventArguments& vec, const void* udata)
			{
				// Convert to lua values
				const sol::reference* ref = (sol::reference*)udata;
				sol::function fnc = (sol::function)(*ref);
				fnc(sol::as_args(vec));
			};

			ev.handler = EventHandler();
			ev.handler.fnc = wrapper;
			auto ref = new sol::reference(fnc);
			ev.handler.user_data = (const void*)ref;
			ev.universe = self;
			ev.signed_up = true;
			ev.ref = ref;
			self->sign_up_for_event(event_id, ev.handler);


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
