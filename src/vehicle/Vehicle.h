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

class Vehicle
{
public:


	std::vector<WeldedGroup*> welded;

	Piece* root;

	std::vector<Piece*> all_pieces;

	// Called almost every frame, as mass can 
	// flow from fuel tanks and similar
	void update_physics();

	// Called when a part separates or joins
	void build_physics(btDynamicsWorld* world);

	// Creates new vehicles from any separated pieces
	std::vector<Vehicle*> handle_separation();

	void draw_debug();

	Vehicle();
	~Vehicle();
};

