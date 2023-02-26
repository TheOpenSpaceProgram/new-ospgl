#pragma once
#include "../LuaCore.h"
#include <universe/Events.h>
#include <util/Logger.h>

struct LuaEventHandler
{
	EventEmitter* emitter;
	EventHandler handler;
	std::string event_id;
	sol::reference* ref;

	bool signed_up;

	void sign_out()
	{
		if(signed_up)
		{
			signed_up = false;
			emitter->drop_out_of_event(event_id, handler);
			delete ref;
		}
	}

	~LuaEventHandler()
	{
		sign_out();
	}

	LuaEventHandler()
	{

	}

	LuaEventHandler(LuaEventHandler&& other)
	{
		emitter = other.emitter;
		handler = other.handler;
		event_id = other.event_id;
		signed_up = other.signed_up;
		ref = other.ref;
		other.signed_up = false;
	}

	LuaEventHandler& operator=(LuaEventHandler&& other)
	{
		emitter = other.emitter;
		handler = other.handler;
		event_id = other.event_id;
		signed_up = other.signed_up;
		ref = other.ref;
		other.signed_up = false;
		return *this;
	}
};

// It's the responsability of the user to store event handlers to prevent their destruction!
class LuaEvents : public LuaLib
{

	virtual void load_to(sol::table& table) override;
};

// Macro used by all event emitters for simpler implementation
// as sol doesn't support fecthing base methods
// Remember to specity EventEmitter in sol bases!
#define EVENT_EMITTER_SIGN_UP(cname) \
		"sign_up_for_event", [](cname* self, const std::string& event_id, sol::protected_function fnc) { \
			LuaEventHandler ev = LuaEventHandler();                                                         \
			ev.event_id = event_id; \
			auto ref = new sol::reference(std::move(fnc)); \
			EventHandlerFnc wrapper = [ref](EventArguments& vec) { \
				sol::protected_function fnc = (sol::protected_function)(*ref); \
				auto result = fnc(sol::as_args(vec)); \
				if(!result.valid()) \
				{ \
					sol::error err = result; \
					LuaUtil::lua_error_handler(fnc.lua_state(), err); \
				} \
			}; \
			ev.handler = EventHandler(); \
			ev.handler.fnc = wrapper; \
			ev.emitter = self;\
			ev.signed_up = true;\
			ev.ref = ref;\
			self->sign_up_for_event(event_id, ev.handler);\
			return std::move(ev); } \

#define EVENT_EMITTER_EMIT(cname)    \
	"emit_event", [](cname* self, const std::string& event_id, sol::variadic_args va) { \
		EventArguments any_vec = EventArguments(); \
		for(auto v : va) {\
			any_vec.push_back(EventArgument(v));}\
		self->emit_event(event_id, any_vec);}                                              \

