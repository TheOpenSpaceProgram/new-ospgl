#pragma once
#pragma warning(push, 0)
#include <BulletDynamics/Dynamics/btRigidBody.h>
#pragma warning(pop)

#include <vector>
#include <unordered_map>
#include <any>

class Part;
class Piece;

struct WeldedGroup
{
	std::vector<Piece*> pieces;
	btRigidBody* rigid_body;
	btMotionState* motion_state;
};


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

	// Position in the current vessel at start of simulation
	btTransform position;

	double mass;

	// Collider shape OF THIS SUBPART
	btCollisionShape* collider;

	// If is_joined_to_others is true then this points
	// to the shared rigidbody, same as motion_state
	btRigidBody* rigid_body;
	btMotionState* motion_state;

	// Used as an offset for rendering, the adjusted
	// position of this collider in the welded shared
	// collider
	btTransform welded_tform;

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
	// nullptr if we are not welded
	WeldedGroup* in_group;

	bool welded;

	btTransform get_current_position();

	Piece();
	~Piece();
};

