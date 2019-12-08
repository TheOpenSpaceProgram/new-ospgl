#include "Piece.h"



btTransform Piece::get_current_position()
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
			return welded_tform * tform;
		}
		else
		{
			return tform;
		}
	}
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
