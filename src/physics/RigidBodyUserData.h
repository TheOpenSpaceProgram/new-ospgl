#pragma once

class Entity;
class Piece;
class WeldedGroup;


enum class RigidBodyType
{
	// Entities can handle rigid body events via the multiple functions there
	ENTITY,
	// Pieces are different, they are handled externally
	PIECE,
	// Welded groups are even more special, the event can be resolved to a particular piece
	WELDED_GROUP,
	// Doesn't have any interaction, purely physical objects
	OTHER
};

// Base class for attaching user data to any rigid body
// allowing advanced picking and physics interaction
struct RigidBodyUserData
{
	union
	{
		Entity* as_ent;
		Piece* as_piece;
		WeldedGroup* as_wgroup;
	};

	RigidBodyType type;

		
	RigidBodyUserData()
	{
		type = RigidBodyType::OTHER;
	}

	RigidBodyUserData(Entity* as_ent)
	{
		type = RigidBodyType::ENTITY;
		this->as_ent = as_ent;
	}

	RigidBodyUserData(WeldedGroup* as_wgroup)
	{
		type = RigidBodyType::WELDED_GROUP;
		this->as_wgroup = as_wgroup;
	}

	RigidBodyUserData(Piece* as_piece)
	{
		type = RigidBodyType::PIECE;
		this->as_piece = as_piece;
	}


};
