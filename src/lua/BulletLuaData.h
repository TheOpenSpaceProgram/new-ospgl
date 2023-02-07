#pragma once
#include <btBulletDynamicsCommon.h>
#include <memory>
#include <sol/sol.hpp>
#include "physics/RigidBodyUserData.h"

// Pointers are held in rigid body to these, for lua usage and
// cleanup
// Mostly used for memory management, but the integrated table acts as
// easy user data!
struct BulletLuaData
{
	btDynamicsWorld* in_world;
	RigidBodyUserData* udata;
	// May be nullptr
	btMotionState* mstate;
	std::shared_ptr<btCollisionShape> shape;
	// Lua may attach stuff here freely
	sol::table assoc_table;

	BulletLuaData(sol::table n_table)
	{
		in_world = nullptr;
		mstate = nullptr;
		assoc_table = n_table;
	}
};
