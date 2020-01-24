#include "SimpleLink.h"

void SimpleLink::activate(btRigidBody* from, btVector3 from_point, btRigidBody* to, btVector3 to_point)
{
	if (constraint == nullptr)
	{
		btTransform frame_a = btTransform::getIdentity();
		btTransform frame_b = btTransform::getIdentity();
		frame_a.setOrigin(from_point);
		frame_b.setOrigin(to_point);

		constraint = new btGeneric6DofSpringConstraint(
			*from, *to, frame_a, frame_b, true);


		constraint->enableSpring(2, true);
		constraint->setStiffness(2, 10000.0);
		constraint->setDamping(2, 0.00005);
		constraint->setBreakingImpulseThreshold(9.0);
		constraint->setLinearUpperLimit(btVector3(0., 0., 10.5));
		constraint->setLinearLowerLimit(btVector3(0., 0., -10.5));

		constraint->setAngularLowerLimit(btVector3(0.f, 0.f, -1.5f));
		constraint->setAngularUpperLimit(btVector3(0.f, 0.f, 1.5f));

		constraint->setDbgDrawSize(btScalar(5.f));


		world->addConstraint(constraint, true);


	}
}

void SimpleLink::deactivate()
{
	if (constraint != nullptr)
	{
		world->removeConstraint(constraint);
		delete constraint;
	}
}

bool SimpleLink::is_broken()
{
	if (constraint == nullptr)
	{
		return false;
	}

	return !constraint->isEnabled();
}

SimpleLink::SimpleLink(btDynamicsWorld* world)
{
	this->world = world;
	constraint = nullptr;
}

SimpleLink::~SimpleLink()
{
	deactivate();
}
