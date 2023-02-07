#pragma once

class Piece;
class WeldedGroup;
class BulletLuaData;

enum class RigidBodyType
{
	// These are present on rigid bodies created in lua
	LUA,
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
		BulletLuaData* as_lua;
		Piece* as_piece;
		WeldedGroup* as_wgroup;
	};

	RigidBodyType type;

		
	RigidBodyUserData()
	{
		type = RigidBodyType::OTHER;
	}

	RigidBodyUserData(BulletLuaData* lua)
	{
		type = RigidBodyType::LUA;
		this->as_lua = lua;
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
