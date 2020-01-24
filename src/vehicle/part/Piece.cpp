#include "Piece.h"
#include "../Vehicle.h"
#include "../../util/Logger.h"

bool Piece::is_welded()
{
	return in_group != nullptr;
}

btTransform Piece::get_global_transform()
{
	logger->check(rigid_body != nullptr, "Part is not added to a vessel");

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
	logger->check(rigid_body != nullptr, "Part is not added to a vessel");

	btVector3 base = rigid_body->getLinearVelocity();
	if (is_welded())
	{
		base += get_tangential_velocity();
	}

	return base;
}

btVector3 Piece::get_angular_velocity()
{
	logger->check(rigid_body != nullptr, "Part is not added to a vessel");

	return rigid_body->getAngularVelocity();

}

btVector3 Piece::get_tangential_velocity()
{
	logger->check(rigid_body != nullptr, "Part is not added to a vessel");

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
	logger->check(rigid_body != nullptr, "Part is not added to a vessel");

	return get_global_transform().getOrigin() - rigid_body->getWorldTransform().getOrigin();
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

	in_vehicle->dirty = true;
}


Piece::Piece(AssetHandle<PartPrototype>&& part_proto, std::string piece_name) 
	: model_node(part_proto->pieces[piece_name].model_node.duplicate())
{
	attached_to = nullptr;
	part = nullptr;
	collider = nullptr;
	rigid_body = nullptr;
	motion_state = nullptr;
	in_group = nullptr;
	welded = false;

	mass = part_proto->pieces[piece_name].mass;
	collider = part_proto->pieces[piece_name].collider;
	collider_offset = part_proto->pieces[piece_name].render_offset;
}

Piece::~Piece()
{
}
