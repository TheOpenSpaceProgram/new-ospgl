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
class Vehicle
{
private:

	bool packed;

	std::unordered_map<int64_t, Piece*> id_to_piece;
	std::unordered_map<int64_t, Part*> id_to_part;

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

	std::vector<Piece*> all_pieces;
	Piece* root;

	// Parts whose root piece is contained in this vehicle
	std::vector<Part*> parts;

	// Bidirectional wires, so if A is connected to B then B is connected to A
	std::unordered_multimap<Machine*, Machine*> wires;

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

	/// Vehicle structure functions:

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