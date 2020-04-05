#pragma once

#pragma warning(push, 0)
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)

#include <sol.hpp>
#include "../../../util/LuaUtil.h"
#include "../../../lua/libs/LuaBullet.h"
#include <cpptoml.h>

// Base class for any link, which can be as simple as
// a bullet3 constraint, or as complex as a soft-body rope
class Link
{
public:

	bool is_initialized;
	sol::state lua_state;

	// Called during loading of the link to give it its 
	// toml serialized data
	void load_toml(std::shared_ptr<cpptoml::table> data)
	{
		LuaUtil::safe_call_function(lua_state,
				"load_toml", "link load_toml", 
				data);
	}

	// Called when the pieces are unwelded, or first created
	void activate(
		btRigidBody* from, btTransform from_frame,
		btRigidBody* to, btTransform to_frame,
		btDynamicsWorld* world
	)
	{
		is_initialized = true;

		LuaUtil::safe_call_function(lua_state,
				"activate", "link activate",
				from, BulletTransform(from_frame), to, BulletTransform(to_frame), world);
	}

	// Called when the pieces are welded
	// Keep in mind special links may have to implement some custom
	// functionality to keep the previous state if they are reactivated,
	// for example, ropes or motors which must remember their last position
	void deactivate()
	{
		is_initialized = false;

		LuaUtil::safe_call_function(lua_state, 
				"deactivate", "link deactivate");
	}

	// Return true if the link has broken and should be deleted
	bool is_broken()
	{
		if(!is_initialized)
		{
			return false;
		}
		else
		{
			return LuaUtil::safe_call_function(lua_state,
				"is_broken", "link is_broken").get<bool>();
		}
	}

	void set_breaking_enabled(bool value)
	{
		LuaUtil::safe_call_function(lua_state,
				"set_breaking_enabled", "link set_breaking_enabled",
				value);
	}

	Link(sol::state&& st)
	{
		this->lua_state = std::move(st);
		is_initialized = false;
	}
};

