#pragma once
#pragma warning(push, 0)
#include <BulletDynamics/Dynamics/btRigidBody.h>
#pragma warning(pop)

#include <vector>
#include <unordered_map>
#include <any>

class Part;

// Pieces are what actually makes up a vehicle
// It has functionality (disabled when we
// are separated from our root), physics and model 
// Note that it may or may not have its own rigid body
// as it may be joined with other pieces into a single one
class Piece
{
public:

	bool is_root;

	// If nullptr it means we are separated from our
	// parent subpart. Always present and has value on
	// root subparts
	Part* part;

	// Position in the current vessel, absolute coordinates
	// On separation it's not readjusted, it's offset doesn't
	// really matter
	btTransform position;

	// Set to true when we are "welded" to other parts
	bool is_joined_to_others;


	double mass;

	// Collider shape OF THIS SUBPART
	btCollisionShape* collider;

	// If is_joined_to_others is true then this points
	// to the shared rigidbody, same as motion_state
	btRigidBody* rigid_body;
	btMotionState* motion_state;

	// Only root pieces can attach to pieces outside
	// of their part, non-root can only attach to other
	// pieces of their part
	// If nullptr, then this part will be separated and 
	// will form no link. This happens, for example,
	// when a decoupler... decouples
	Piece* attached_to;

	// If a part is welded, then its collider is shared
	// with other pieces, being welded
	// Otherwise there will be a link between it and its 
	// attached_to part, this can be a constraint or any
	// other thing (maybe wires or ropes?)
	bool is_welded;

	Piece();
	~Piece();
};

