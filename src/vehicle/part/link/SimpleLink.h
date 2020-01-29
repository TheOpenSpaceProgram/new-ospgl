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
		btRigidBody* from, btTransform from_frame,
		btRigidBody* to, btTransform to_frame
	) override;

	// Called when the pieces are welded or the link dies
	virtual void deactivate() override;

	// Return true if the link has broken and should be deleted
	// Keep in mind this returns false if the link is non-existant!
	virtual bool is_broken() override;

	virtual void set_breaking_enabled(bool value);

	SimpleLink(btDynamicsWorld* world);
	~SimpleLink();
};

