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

#include "UnpackedVehicle.h"
#include "PackedVehicle.h"


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
class Vehicle : public Drawable
{
private:

	bool packed;
	
	// Initialized on the call to init, is deleted afterwards
	std::shared_ptr<cpptoml::table_array> wires_init;

	std::unordered_map<int64_t, Piece*> id_to_piece;
	std::unordered_map<int64_t, Part*> id_to_part;

public:

	Universe* in_universe;

	UnpackedVehicle unpacked_veh;
	PackedVehicle packed_veh;
	
	friend class UnpackedVehicle;
	friend class PackedVehicle;
	friend class VehicleLoader;
	friend class Piece;
	friend class Part;

	int64_t part_id;
	int64_t piece_id;	
	
	std::vector<Piece*> all_pieces;
	Piece* root;

	// Parts whose root piece is contained in this vehicle
	std::vector<Part*> parts;
	// Keep up to date by the machines themselves
	std::vector<Port*> all_ports;

	virtual void deferred_pass(CameraUniforms& camera_uniforms) override;
	virtual bool needs_deferred_pass() override { return true; }

	virtual void shadow_pass(ShadowCamera& sh_camera) override;
	virtual bool needs_shadow_pass() override { return true; }

	void pack();
	void unpack();
	bool is_packed() { return packed; }

	void set_position(glm::dvec3 pos);
	void set_linear_velocity(glm::dvec3 vel);

	void update(double dt);	

	// The return is only populated if something separated
	std::vector<Vehicle*> physics_update(double pdt);
	
	// Called when the vehicle is added to the world
	void init(Universe* into_universe);

	void set_world(btDynamicsWorld* world)
	{	
		unpacked_veh.world = world;
	}

	void sort();

	// Removes and reports any wrong wires (as an error)
	void check_wires();

	Piece* get_piece(int64_t id);
	Part* get_part(int64_t id);

	Vehicle();
	~Vehicle();
};

