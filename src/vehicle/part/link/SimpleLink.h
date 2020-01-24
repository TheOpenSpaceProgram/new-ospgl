#pragma once
#include "Link.h"


class SimpleLink : public Link
{
private:

	btGeneric6DofSpringConstraint* constraint;
	btDynamicsWorld* world;

public:

	// Called when the pieces are unwelded, or first created
	virtual void activate(
		btRigidBody* from, btVector3 from_point,
		btRigidBody* to, btVector3 to_point
	) override;

	// Called when the pieces are welded
	virtual void deactivate() override;

	// Return true if the link has broken and should be deleted
	virtual bool is_broken() override;

	SimpleLink(btDynamicsWorld* world);
	~SimpleLink();
};

