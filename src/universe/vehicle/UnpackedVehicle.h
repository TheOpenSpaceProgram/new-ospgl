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
	void handle_piece_separated(Piece* p);

public:
	struct PieceState
	{
		btTransform transform;
		btVector3 linear;
		btVector3 linear_tang;
		btVector3 angular;
	};
	Vehicle* vehicle;

	btDynamicsWorld* world;

	bool dirty;

	std::vector<WeldedGroup*> welded;
	std::vector<Piece*> single_pieces;
	

	// Call every frame, it checks the dirty flag
	// Can create new vehicles if parts separate (only when unpacked)
	void update();

	// Called automatically by update to rebuild the physics
	// whenever the dirty flag is set
	void build_physics(std::unordered_map<Piece*, PieceState>& states_at_start);

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

	// Optionally returns rendering center of mass, for camera focusing
	glm::dvec3 get_center_of_mass(bool renderer = false);

	// Returns velocity of root part
	glm::dvec3 get_velocity();

	// Returns orientation of the root part
	glm::dquat get_orientation(bool renderer);

	void set_breaking_enabled(bool value);

	void set_world(btDynamicsWorld* n_world)
	{
		this->world = n_world;
	}

	void deactivate();
	void activate();

	void apply_gravity(btVector3 direction);

	UnpackedVehicle(Vehicle* v);

};
