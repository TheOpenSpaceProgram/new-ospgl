#pragma once
#include "part/Part.h"
#include "part/Piece.h"


#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)

#include <unordered_set>
#include <vector>
class Vehicle;

class UnpackedVehicle
{
public:

private:

	bool breaking_enabled;
public:
	Vehicle* vehicle;

	btDynamicsWorld* world;

	bool dirty;

	std::vector<WeldedGroup*> welded;
	std::vector<Piece*> single_pieces;
	

	// Call every frame, it checks the dirty flag
	// Can create new vehicles if parts separate (only when unpacked)
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

	glm::dvec3 get_center_of_mass();

	// Orders the all_pieces array so that parts are ordered
	// from (tree) distance to root
	void sort();

	void set_breaking_enabled(bool value);

	void set_world(btDynamicsWorld* n_world)
	{
		this->world = n_world;
	}

	void deactivate();
	void activate();

	UnpackedVehicle(Vehicle* v);

};
