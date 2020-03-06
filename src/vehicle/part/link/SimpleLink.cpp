#include "SimpleLink.h"

void SimpleLink::activate(btRigidBody* from, btTransform from_frame, btRigidBody* to, btTransform to_frame)
{
	if (constraint == nullptr)
	{

		constraint = new btGeneric6DofSpringConstraint(
			*from, *to, from_frame, to_frame, true);


		constraint->enableSpring(2, true);
		constraint->setStiffness(2, 10000.0);
		constraint->setDamping(2, 0.000005);
		constraint->setBreakingImpulseThreshold(30.0);
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

		constraint = nullptr;
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

void SimpleLink::set_breaking_enabled(bool value)
{
	if (constraint)
	{
		constraint->setBreakingImpulseThreshold(value ? 30.0 : 100000000.0);
	}
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
