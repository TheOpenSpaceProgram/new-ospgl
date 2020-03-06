#pragma once

#pragma warning(push, 0)
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)

// Base class for any link, which can be as simple as
// a bullet3 constraint, or as complex as a soft-body rope
class Link
{
public:

	// Called when the pieces are unwelded, or first created
	virtual void activate(
		btRigidBody* from, btTransform from_frame,
		btRigidBody* to, btTransform to_frame
	) = 0;

	// Called when the pieces are welded
	// Keep in mind special links may have to implement some custom
	// functionality to keep the previous state if they are reactivated,
	// for example, ropes or motors which must remember their last position
	virtual void deactivate() = 0;

	// Return true if the link has broken and should be deleted
	virtual bool is_broken() = 0;

	virtual void set_breaking_enabled(bool value) = 0;
};

