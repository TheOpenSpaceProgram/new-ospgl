#pragma once
#include "../LuaLib.h"
#include <universe/Universe.h>

// So that the lua user doesn't have to manage
// the lifetime of events
// Regardless: the lua user must make sure the event handlers are stored somewhere
// so they are not discarded by the GC
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
