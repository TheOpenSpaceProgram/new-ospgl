#include "Piece.h"



btTransform Piece::get_current_transform()
{
	if (motion_state == nullptr)
	{
		return position;
	}
	else
	{
		btTransform tform;


		//motion_state->getWorldTransform(tform);

		tform = rigid_body->getWorldTransform();

		if (in_group != nullptr)
		{
			return tform * welded_tform;
		}
		else
		{
			return tform;
		}
	}
}

btVector3 Piece::get_current_linear()
{
	if (rigid_body == nullptr)
	{
		return btVector3(0.0, 0.0, 0.0);
	}
	else
	{
		btVector3 base = rigid_body->getLinearVelocity();
		if (in_group != nullptr)
		{
			// We have to add the linear component that we may have
			// got from rotation
			btVector3 r = get_current_transform().getOrigin() - rigid_body->getWorldTransform().getOrigin();
			btVector3 linear = rigid_body->getAngularVelocity().cross(r);
			base += linear;
		}

		return base;
	}
}

btVector3 Piece::get_current_angular()
{
	if (rigid_body == nullptr)
	{
		return btVector3(0.0, 0.0, 0.0);
	}
	else
	{
		return rigid_body->getAngularVelocity();
		/*btVector3 rv = get_relative_position();
		btVector3 pv = mass * (get_current_linear() - rigid_body->getLinearVelocity());

		return rv.cross(pv) / mass;*/
	}
}

btVector3 Piece::get_relative_position()
{
	return get_current_transform().getOrigin() - rigid_body->getWorldTransform().getOrigin();
}

Piece::Piece()
{
	attached_to = nullptr;
	part = nullptr;
	collider = nullptr;
	rigid_body = nullptr;
	motion_state = nullptr;
	in_group = nullptr;

	welded = false;
}


Piece::~Piece()
{
}
