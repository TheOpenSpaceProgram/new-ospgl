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
		btRigidBody* from, btVector3 from_point,
		btRigidBody* to, btVector3 to_point
	) = 0;

	// Called when the pieces are welded
	virtual void deactivate() = 0;

	// Return true if the link has broken and should be deleted
	virtual bool is_broken() = 0;
};

