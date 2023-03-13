#pragma once
#include "part/Part.h"
#include "part/Piece.h"

#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)

#include <unordered_set>
#include <vector>
#include <cpptoml.h>

#include <renderer/Drawable.h>

#include "UnpackedVehicle.h"
#include "PackedVehicle.h"
#include "plumbing/VehiclePlumbing.h"
#include <universe/Events.h>
#include "VehicleMeta.h"


class VehicleLoader;

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
// Takes ownership of all pieces, parts, machines...
// They must be allocated in the heap because we delete them
//
// Emits the following events:
// on_separate_vehicle(ent_id: integer) - Note: vehicle entity must implement separate_vehicle(veh: vehicle)
//		Called when any piece is separated AND FORMS A NEW VEHICLE
//		This is called after the vehicle is created, so you can use the returned entity
//
// on_separate_piece(piece: integer)
// 		Called when any piece is lost from the vehicle by separation
// 		If multiple pieces are lost, it's called for each one of them
// 		This is called while the piece is still in the current vehicle (but not attached!)
//
// on_lost_piece(piece: integer)
// 		Called when any piece is lost from the vehicle, be it by separation or destruction
// 		If multiple pieces are lost, it's called for each one of them
// 		This is called while the piece is still in the current vehicle (but not attached!)
//
// on_destroyed_piece(piece: integer)
// 		Called when a piece is lost from the vehicle by destruction
// 		If multiple pieces are lost, it's called for each one of them
// 		This is called while the piece is still in the current vehicle, before deletion
//
// on_sort()
//		Called BEFORE the vehicle is sorted, indices will be invalidated
//
// after_sort()
// 		Called AFTER the vehicle is sorted
//
// Note: As piece / part IDs are unique in the whole universe, you may track separated pieces / parts with the
// same ID as before, just search for them in separated vehicles. Storing direct pointers to parts / pieces is fine
// as long as these are not destroyed. That's what on_destroyed_piece is for
class Vehicle : public EventEmitter
{
private:

	bool packed;

	std::unordered_map<int64_t, Piece*> id_to_piece;
	std::unordered_map<int64_t, Part*> id_to_part;

	// Clones a piece, creating a part for it if neccesary (or returning already created)
	// but doesn't clone attachments, wiring or anything similar
	Piece* clone_piece(Piece* p, std::vector<Part*>& seen_parts, std::vector<Part*>& created_parts);

public:

	Universe* in_universe;
	Entity* in_entity;

	UnpackedVehicle unpacked_veh;
	PackedVehicle packed_veh;
	VehiclePlumbing plumbing;
	
	friend class UnpackedVehicle;
	friend class PackedVehicle;
	friend class VehicleLoader;
	friend class Piece;
	friend class Part;

	VehicleMeta meta;

	std::vector<Piece*> all_pieces;
	Piece* root;

	// Parts whose root piece is contained in this vehicle
	std::vector<Part*> parts;

	// Bidirectional wires, so if A is connected to B then B is connected to A
	std::unordered_multimap<Machine*, Machine*> wires;

	// These return nullptr if the part / piece is no longer present in this vehicle
	Part* get_part_by_id(int64_t id);
	Piece* get_piece_by_id(int64_t id);

	void pack();

	void unpack();
	bool is_packed() const { return packed; }

	void set_position(glm::dvec3 pos);
	void set_linear_velocity(glm::dvec3 vel);

	void update(double dt);	
	void editor_update(double dt);
	
	// Used when a piece is totally destroyed, the 
	// piece is not actually memory deleted, so you 
	// get the pointer back
	// Remember to update the vehicle if on flight
	Piece* remove_piece(Piece* p);

	void physics_update(double pdt);
	
	// Called when the vehicle is added to the world
	void init(Universe* into_universe, Entity* in_entity);
	// For the editor
	void init(sol::state* lua_state);

	// Finds which attachments are used on which parts
	// (The vehicle doesn't need to be sorted)
	void update_attachments();

	void set_world(btDynamicsWorld* world)
	{	
		unpacked_veh.world = world;
	}

	void set_world(btDiscreteDynamicsWorld* world)
	{
		unpacked_veh.world = world;
	}

	void sort();
	void remove_outdated();

	// Gets the bounds of the vehicle relative to the root piece (which is used for positioning)
	// in "editor / vehicle" coordinates.
	// Returns lower bound (whose z value is minimum and represents the floor of the vehicle)
	// and the upper bound (opposite of the floor, whose z value is maximum)
	// Note: Bounds are calculated using colliders!
	std::pair<glm::dvec3, glm::dvec3> get_bounds();

	// Vehicle structure functions:

	// This one doesn't need the array to be sorted
	// but it's slower (used in the editor)
	// It allows finding children of separated parts
	std::vector<Piece*> get_children_of(Piece* p);

	// Does not include what p is attached to!
	std::vector<Piece*> get_attached_to(Piece* p);
	// This one does include wathever p is attached to
	std::vector<Piece*> get_connected_to(Piece* p);
	// This potentially includes p if it's attached through said marker!
	Piece* get_connected_with(Piece* p, const std::string& attachment_marker);

	// Duplicates a piece, and all its children, doing a deep copy of values, wiring
	// and plumbing, but only of data within the given piece and children
	Piece* duplicate(Piece* p);

	Vehicle();
	~Vehicle();
};

template<>
class GenericSerializer<Vehicle>
{
public:

	static void serialize(const Vehicle& what, cpptoml::table& target);
	static void deserialize(Vehicle& to, const cpptoml::table& from);
};