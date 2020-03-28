#include "Piece.h"
#include "../Vehicle.h"
#include "../../util/Logger.h"

bool Piece::is_welded()
{
	return in_group != nullptr;
}

btTransform Piece::get_global_transform(bool use_mstate)
{
	if(in_vehicle->is_packed())
	{
		return packed_tform;
	}
	else 
	{
		btTransform tform;

		if (use_mstate)
		{
			motion_state->getWorldTransform(tform);
		}
		else
		{
			tform = rigid_body->getWorldTransform();
		}

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

btVector3 Piece::get_linear_velocity(bool ignore_tangential)
{
	if(in_vehicle->is_packed())
	{
		return to_btVector3(in_vehicle->packed_veh.root_state.cartesian.vel);
	}
	else
	{
		btVector3 base = rigid_body->getLinearVelocity();
		if (is_welded() && !ignore_tangential)
		{
			base += get_tangential_velocity();
		}

		return base;
	}
}

btVector3 Piece::get_angular_velocity()
{
	if(in_vehicle->is_packed())
	{
		// TODO
		return btVector3(0, 0, 0);	
	}
	else 
	{
		return rigid_body->getAngularVelocity();
	}
}

btVector3 Piece::get_tangential_velocity()
{

	if(in_vehicle->is_packed())
	{
		return btVector3(0.0, 0.0, 0.0);
	}
	else 
	{

		if (!is_welded())
		{
			return btVector3(0.0, 0.0, 0.0);
		}

		btVector3 r = get_relative_position();
		btVector3 tangential = rigid_body->getAngularVelocity().cross(r);
	
		return tangential;
	}
}

btVector3 Piece::get_relative_position()
{
	if(in_vehicle->is_packed())
	{
		//TODO:
		return btVector3(0, 0, 0);
	}
	else
	{		
		return get_global_transform().getOrigin() - rigid_body->getWorldTransform().getOrigin();
	}
}	

void Piece::set_dirty()
{
	if (in_group != nullptr)
	{
		in_group->dirty = true;
	}
	else
	{
		dirty = true;
	}

	in_vehicle->unpacked_veh.dirty = true;
}


Piece::Piece(Part* in_part, std::string piece_name) 
	: model_node(in_part->part_proto->pieces[piece_name].model_node.duplicate())
{
	attached_to = nullptr;
	part = nullptr;
	collider = nullptr;
	rigid_body = nullptr;
	motion_state = nullptr;
	in_group = nullptr;
	welded = false;

	part = in_part;

	mass = part->part_proto->pieces[piece_name].mass;
	friction = part->part_proto->pieces[piece_name].friction;
	restitution = part->part_proto->pieces[piece_name].restitution;

	collider = part->part_proto->pieces[piece_name].collider;
	collider_offset = part->part_proto->pieces[piece_name].render_offset;
}

Piece::~Piece()
{
}
