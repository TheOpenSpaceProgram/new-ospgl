#include "Piece.h"
#include "../Vehicle.h"
#include <util/Logger.h>

glm::dmat4 Piece::get_graphics_matrix()
{
	return to_dmat4(get_global_transform()) * glm::inverse(collider_offset);
}

bool Piece::is_welded()
{
	return in_group != nullptr;
}

btTransform Piece::get_graphics_transform()
{
	return get_global_transform_internal(true);
}

btTransform Piece::get_global_transform()
{
	return get_global_transform_internal(false);
}

btTransform Piece::get_global_transform_internal(bool use_mstate)
{
	if(in_vehicle->is_packed())
	{
		return in_vehicle->packed_veh.get_root_transform() * packed_tform;
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
		btVector3 base = to_btVector3(in_vehicle->packed_veh.get_root_state().vel);
	
		base += get_tangential_velocity();

		return base;
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
		return to_btVector3(in_vehicle->packed_veh.get_root_state().ang_vel);
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
		btVector3 r = packed_tform.getOrigin();
		// We need to correct r to the center of mass
		btVector3 com = in_vehicle->packed_veh.get_com_root_relative();
		r -= com;
		btVector3 tangential = get_angular_velocity().cross(r);
		return tangential;
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
	// TODO: Check this stuff
	//return get_local_transform().getOrigin();
	if(is_welded())
	{
		// Not get_global_transform() as that already includes get_local_transform()
		btTransform global = rigid_body->getWorldTransform();
		btTransform local = get_local_transform();
		btTransform mul = global * local; //< This is final position

		btVector3 final = mul.getOrigin() - global.getOrigin();

		return final;
	}
	else
	{
		return btVector3(0, 0, 0);
	}
}	 

void Piece::set_dirty(bool update_now)
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

	if(update_now)
	{
		in_vehicle->unpacked_veh.update();
	}
}


std::pair<PieceAttachment, bool>* Piece::find_attachment(std::string marker_name) 
{
	for(auto& pair : attachments)
	{
		if(pair.first.marker == marker_name)
		{
			return &pair;
		}
	}	

	return nullptr;
}

glm::dvec3 Piece::transform_axis(glm::dvec3 axis)
{
	btQuaternion rot = get_global_transform().getRotation();
	glm::dvec3 result = glm::toMat4(to_dquat(rot)) * glm::dvec4(axis, 1.0);

	return result;	
}

glm::dvec3 Piece::get_forward()
{
	return transform_axis(glm::dvec3(0, 0, 1));	
}

glm::dvec3 Piece::get_up()
{
	return transform_axis(glm::dvec3(0, 1, 0));
}

glm::dvec3 Piece::get_right()
{
	return transform_axis(glm::dvec3(1, 0, 0));
}

Marker& Piece::get_marker(const std::string & marker_name)
{
	auto it = markers.find(marker_name);
	logger->check(it != markers.end(), "Could not find marker named '{}'", marker_name);
	return it->second;
}

glm::dvec3 Piece::get_marker_position(const std::string & marker_name)
{
	return get_marker(marker_name).origin;
}

glm::dquat Piece::get_marker_rotation(const std::string& marker_name)
{
	return get_marker(marker_name).rotation;
}

glm::dmat4 Piece::get_marker_transform(const std::string & marker_name)
{
	return get_marker(marker_name).transform;
}

glm::dvec3 Piece::get_marker_forward(const std::string & marker_name)
{
	return get_marker(marker_name).forward;
}

glm::dvec3 Piece::transform_point_to_rigidbody(glm::dvec3 p)
{
	glm::dvec3 f = glm::dvec3(to_dmat4(get_global_transform()) * glm::dvec4(p, 1.0));
	f -= to_dvec3(rigid_body->getCenterOfMassPosition());
	
	return f;
}

Piece::Piece(Part* in_part, std::string piece_name)
	: model_node(in_part->part_proto->pieces.at(piece_name).model_node.duplicate())
{
	attached_to = nullptr;
	part = nullptr;
	collider = nullptr;
	rigid_body = nullptr;
	motion_state = nullptr;
	in_group = nullptr;
	welded = false;

	part = in_part;

	piece_prototype = &in_part->part_proto.get_noconst()->pieces[piece_name];

	mass = piece_prototype->mass;
	friction = piece_prototype->friction;
	restitution = piece_prototype->restitution;

	collider = piece_prototype->collider;
	collider_offset = piece_prototype->render_offset;

	markers = piece_prototype->markers;
	for(PieceAttachment atc : piece_prototype->attachments)
	{
		attachments.push_back(std::make_pair(atc, false));
	}

	editor_dettachable = piece_prototype->editor_dettachable;

}

Piece::~Piece()
{
	// TODO: Automatic removal of the part if we are root piece
}

double Piece::get_environment_pressure()
{
	return 0;
}

glm::dmat4 Piece::get_in_vehicle_matrix()
{
	if(in_vehicle->is_packed())
	{
		return to_dmat4(packed_tform);
	}
	else
	{
		// We get the root transform and our transform to obtain the matrix
		auto our_tform = to_dmat4(get_global_transform());
		auto root_tform = to_dmat4(in_vehicle->root->get_global_transform());
		return glm::inverse(our_tform) * root_tform;
	}
}
