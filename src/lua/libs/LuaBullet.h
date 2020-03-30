#pragma once
#include "../LuaLib.h"
#include "../../util/Logger.h"
#include "../../physics/glm/BulletGlmCompat.h"

struct BulletTransform
{
	glm::dvec3 pos;
	glm::dquat rot;

	BulletTransform(btTransform tf)
	{
		pos = to_dvec3(tf.getOrigin());
		rot = to_dquat(tf.getRotation());
	}

	BulletTransform(glm::dvec3 p)
	{
		pos = p;
		rot = glm::dquat(1.0, 0.0, 0.0, 0.0);
	}

	BulletTransform(glm::dvec3 p, glm::dquat q)
	{
		pos = p;
		rot = q;
	}

	btTransform to_btTransform()
	{
		btTransform out;
		out.setOrigin(to_btVector3(pos));
		out.setRotation(to_btQuaternion(rot));
		return out;
	}

	glm::dmat4 to_dmat4()
	{
		return ::to_dmat4(to_btTransform());
	}
};

// 
// Unlike in C++, lua never sees the internal
// bullet geometric classes, they always use glm
// This means that the API is not a direct translation,
// but has some differences:
// - We use BulletTransform instead of the bullet transform class
// 	 to use glm types
// - DynamicsWorld is not implemented, but you can add stuff to 
//   it from the different classes (rigid_body:add_to_world(bt_world))
// - RigidBody is named rigidbody and not rigid_body (it's used a lot)
//
class LuaBullet : public LuaLib
{
public:

	virtual void load_to(sol::table& table) override;	

};
