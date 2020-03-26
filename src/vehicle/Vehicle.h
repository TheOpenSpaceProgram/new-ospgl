#pragma once
#include "part/Part.h"
#include "part/Piece.h"


#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)

#include <unordered_set>
#include <vector>
#include <cpptoml.h>

#include "../renderer/Drawable.h"

// A Vehicle is basically a tree of parts (actually pieces), connected
// via various links. The root piece is always the root node.
// Parts are made of one or multiple pieces (physic units),
// for example, a fuel tank may have a single piece, with
// all link points originating from it, while a robotic hinge
// may have 2 pieces, one fixed to the vessel and the other
// rotating. Links are created between pieces so the physics
// system can properly simulate cool stuff
// Decouplers are another good example, they contain two pieces
// that can be broken on user command. 
// ---
// Does not allow circular dependencies, so docking may be a bit tricky,
// but this is exactly the same as KSP's mechanic. Maybe the
// non-main docking ports could simply create a very strong link, while
// the main one actually allows welding.
// ---
// Packing:
//
// 	Vehicles can exist in two states, loaded into the physics world and simulated
// 	with bullet (unpacked), or being simulated by trajectories, without part
// 	physics (packed).
//  While the vehicle is packed, all pieces have the same velocity, but can have 
//  different positions. Pieces store their absolute position, but the trajectory
//  updates the center of mass of the vehicle. 
//
// 	Vehicles are always serialized as packed, which means that serialization
// 	is only possible when the vehicle is relatively estabilized, without very big
// 	inter-part interactions. A similar situation happens in the vehicle editor.
//
class Vehicle : public Drawable
{
private:

	bool breaking_enabled;
	bool packed;
public:

	int64_t part_id;
	int64_t piece_id;	

	btDynamicsWorld* world;

	bool dirty;

	std::vector<WeldedGroup*> welded;

	Piece* root;

	std::vector<Piece*> all_pieces;
	std::vector<Piece*> single_pieces;
	
	// Parts whose root piece is contained in this vehicle
	std::vector<Part*> parts;

	// Call every frame, it checks the dirty flag
	// Can create new vehicles if parts separate
	std::vector<Vehicle*> update();

	// Called automatically by update to rebuild the physics
	// whenever the dirty flag is set
	void build_physics();

	// Piece gets 0 velocity and angular momentum
	// Use only while building the vehicle, all at once
	void add_piece(Piece* piece, btTransform pos);

	// Creates new vehicles from any separated pieces
	// (that cannot reach the root piece)
	// It automatically assigns Parts, pieces, ids...
	std::vector<Vehicle*> handle_separation();

	void draw_debug();

	// The root part ends up in the given position, other parts
	// keep their relative position
	void set_position(glm::dvec3 pos);
	// Relative velocities are properly kept
	void set_linear_velocity(glm::dvec3 vel);

	// Orders the all_pieces array so that parts are ordered
	// from (tree) distance to root
	void sort();

	void set_breaking_enabled(bool value);

	virtual void deferred_pass(CameraUniforms& camera_uniforms) override;
	virtual bool needs_deferred_pass() override { return true; }

	void set_world(btDynamicsWorld* n_world)
	{
		this->world = n_world;
	}

	// Unloads physics, you must keep updating the vehicle via a Trajectory
	void pack();
	// Reloads physics, starting from the last received position and packed positions
	void unpack();

	bool is_packed()
	{
		return packed;
	}

	Vehicle();
	~Vehicle();
};

