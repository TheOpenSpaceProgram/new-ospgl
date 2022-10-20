#include "LuaUniverse.h"

#include <utility>
#include <universe/entity/Entity.h>

void LuaUniverse::load_to(sol::table& table)
{
	table.new_usertype<LuaEventHandler>("lua_event_handler",
		"sign_out", &LuaEventHandler::sign_out
	);

	table.new_usertype<Universe>("universe",
		"sign_up_for_event", [](Universe* self, const std::string& event_id, sol::protected_function fnc)
		{
			LuaEventHandler ev = LuaEventHandler();

			ev.event_id = event_id;

			EventHandlerFnc wrapper = [](EventArguments& vec, const void* udata)
			{
				// Convert to lua values
				const sol::reference* ref = (sol::reference*)udata;
				sol::protected_function fnc = (sol::protected_function)(*ref);
				// Handle errors, otherwise debugging could get very confusing!
				auto result = fnc(sol::as_args(vec));
				if(!result.valid())
				{
					sol::error err = result;
					LuaUtil::lua_error_handler(fnc.lua_state(), err);
				}
			};

			ev.handler = EventHandler();
			ev.handler.fnc = wrapper;
			auto ref = new sol::reference(std::move(fnc));
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
		},
		"bt_world", &Universe::bt_world,
		"system", &Universe::system,
		// We implement a getter, to modify entities use the given functions
		"entities", sol::property([](Universe* uv)
		  {
			return sol::as_table(uv->entities);
		  }),
	    "create_entity", [](Universe* uv, const std::string& script_path, sol::this_environment te, sol::variadic_args args)
		 {
			sol::environment& env = te;
			 // Not sure if this is neccesary, maybe just pass args?
			 std::vector<sol::object> args_v;
			 for(auto v : args)
			 {
				 args_v.push_back(v);
			 }
			auto* ent = uv->create_entity<Entity>(script_path,
				   env["__pkg"].get_or<std::string>("core"), nullptr, args_v, true);

			return ent;
		 }
	);

	table.new_usertype<PlanetarySystem>("planetary_system", sol::base_classes, sol::bases<Drawable>());
	table.new_usertype<Entity>("entity", sol::no_constructor, sol::base_classes, sol::bases<Drawable>(),
	        "enable_bullet", &Entity::enable_bullet,
	        "disable_bullet", &Entity::disable_bullet,
	        "set_bullet_enabled", &Entity::enable_bullet_wrapper,
	        "get_physics_origin", &Entity::get_physics_origin,
	        "get_physics_radius", &Entity::get_physics_radius,
	        "get_visual_origin", &Entity::get_visual_origin,
	        "is_physics_loader", &Entity::is_physics_loader,
	        "timewarp_safe", &Entity::timewarp_safe,
	        "uid", sol::property(&Entity::get_uid),
	        "get_type", &Entity::get_type,
	        "save", &Entity::save,
	        "drawable_uid", sol::readonly(&Entity::drawable_uid));

}
