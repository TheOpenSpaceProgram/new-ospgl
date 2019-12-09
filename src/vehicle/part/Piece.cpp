#include "Piece.h"



bool Piece::is_welded()
{
	return in_group != nullptr;
}

btTransform Piece::get_global_transform()
{
	if (rigid_body == nullptr)
	{
		return position;
	}
	else
	{
		btTransform tform;

		//motion_state->getWorldTransform(tform);
		tform = rigid_body->getWorldTransform();

		if (is_welded())
		{
			return tform * welded_tform;
		}
		else
		{
			return tform;
		}
	}
}

btTransform Piece::get_local_transform()
{
	if (is_welded())
	{
		return welded_tform;
	}
	else
	{
		return btTransform::getIdentity();
	}
	
}

btVector3 Piece::get_linear_velocity()
{
	if (rigid_body == nullptr)
	{
		return btVector3(0.0, 0.0, 0.0);
	}
	else
	{
		btVector3 base = rigid_body->getLinearVelocity();
		if (is_welded())
		{
			base += get_tangential_velocity();
		}

		return base;
	}
}

btVector3 Piece::get_angular_velocity()
{
	if (rigid_body == nullptr)
	{
		return btVector3(0.0, 0.0, 0.0);
	}
	else
	{
		return rigid_body->getAngularVelocity();
	}
}

btVector3 Piece::get_tangential_velocity()
{
	if (!is_welded())
	{
		return btVector3(0.0, 0.0, 0.0);
	}

	btVector3 r = get_relative_position();
	btVector3 tangential = rigid_body->getAngularVelocity().cross(r);
	
	return tangential;
}

btVector3 Piece::get_relative_position()
{
	return get_global_transform().getOrigin() - rigid_body->getWorldTransform().getOrigin();
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
