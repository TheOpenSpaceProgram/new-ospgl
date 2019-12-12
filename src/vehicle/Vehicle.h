#pragma once
#include "part/Part.h"
#include "part/Piece.h"


#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)

#include <unordered_set>
#include <vector>

// A Vehicle is basically a tree of parts (actually subparts), connected
// via various links. The root part is always the root node.
// Parts can be formed by various subparts (physic units),
// for example, a fuel tank may have a single subpart, with
// all link points originating from it, while a robotic hinge
// may have 2 subparts, one fixed to the vessel and the other
// rotating. Links are created between subparts so the physics
// system can properly simulate cool stuff
// Decouplers are another good example, they contain two subparts
// that can be broken on user command. 
// ---
// Does not allow circular dependencies, so docking may be a bit tricky,
// but this is exactly the same as KSP's mechanic. Maybe the non
// principal docking ports could simply create a very strong link, while
// the principal one actually allows welding.
class Vehicle
{
private:


public:

	btDynamicsWorld* world;

	bool dirty;

	std::vector<WeldedGroup*> welded;

	Piece* root;

	std::vector<Piece*> all_pieces;
	std::vector<Piece*> single_pieces;

	// Call every frame, it checks the dirty flag
	// Can create new vehicles if parts separate
	void update(std::vector<Vehicle*>& vehicles);

	// Called when a part separates or joins, or the type of
	// link changes, so the physics engine can recreate the
	// rigid bodies.
	void build_physics();

	// Piece gets 0 velocity and angular momentum
	// Use only while building the vehicle, all at once
	void add_piece(Piece* piece, btTransform pos);

	// Creates new vehicles from any separated pieces
	// (that cannot reach the root piece)
	std::vector<Vehicle*> handle_separation();

	void draw_debug();

	void set_position(btVector3 pos);
	void set_linear_velocity(btVector3 vel);

	// Orders the all_pieces array so that parts are ordered
	// from distance to root
	void sort();

	Vehicle(btDynamicsWorld* world);
	~Vehicle();
};

