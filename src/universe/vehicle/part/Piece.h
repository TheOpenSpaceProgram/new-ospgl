#pragma once
#pragma warning(push, 0)
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#pragma warning(pop)

#include <vector>
#include <unordered_map>
#include <any>
#include <memory>

#include "Link.h"
#include <assets/PartPrototype.h>



class Part;
class Piece;
class Vehicle;

struct WeldedGroup
{
	std::vector<Piece*> pieces;
	btRigidBody* rigid_body;
	btMotionState* motion_state;

	// Should the rigidbody be rebuilt?
	bool dirty;
};


// Pieces are what actually makes up a vehicle
// It has functionality (disabled when we
// are separated from our root), physics and model 
// Note that it may or may not have its own rigid body
// as it may be joined with other pieces into a single one
// A lot of functions return bullet vectors as they are mostly
// used there.
class Piece
{
private:

	bool dirty;
	
	btTransform get_global_transform_internal(bool use_mstate);

public:
	
	// Transform used while the piece is packed
	// (aka, physics simulation is off)
	// They are root relative coordinates
	btTransform packed_tform;

	int64_t id;

	GPUModelNodePointer model_node;
	// We have to keep it loaded! This is the easiest way
	AssetHandle<PartPrototype> part_prototype;

	Vehicle* in_vehicle;

	// If nullptr it means we are separated from our
	// parent subpart. Always has value on
	// root subparts
	Part* part;

	double mass;

	double friction;
	double restitution;

	// Collider shape OF THIS SUBPART
	// If you want a advanced compound collider with moving
	// parts (for example, a fuel tank with simulated "liquid" 
	// fuel, that's free to move), create another Piece
	// which is always welded. That works better. (TODO: Check perfomance)
	btCollisionShape* collider;

	// If is_joined_to_others is true then this points
	// to the shared rigidbody, same as motion_state
	btRigidBody* rigid_body;
	btMotionState* motion_state;

	// Used as an offset for rendering, the adjusted
	// position of this collider in the welded shared
	// collider
	btTransform welded_tform;

	// Collider offset relative to the piece node, used for rendering
	glm::dmat4 collider_offset;

	// Only root pieces can attach to pieces outside
	// of their part, non-root can only attach to other
	// pieces of their part
	// If nullptr, then this part will be separated and 
	// will form no link. This happens, for example,
	// when a decoupler... decouples
	Piece* attached_to;

	// If a part is welded, then its collider is shared
	// with other pieces
	// Otherwise there MAY be a link between it and its 
	// attached_to part, this can be a constraint or any
	// other thing (maybe wires or ropes?)
	// nullptr if we are not welded
	WeldedGroup* in_group;
	bool welded;
	// Negative if not welded
	int welded_collider_id;

	// Guaranteed to get the actual welded state, even if the
	// user sets "welded" to false and "build_physics" has not
	// yet been called. Mostly used internally
	bool is_welded();

	// This function uses the motion state, unlike get_global_transform
	btTransform get_graphics_transform();
	btTransform get_global_transform();
	btTransform get_local_transform();
	btVector3 get_linear_velocity(bool ignore_tangential = false);
	btVector3 get_angular_velocity();
	// Returns zero on non-welded pieces
	btVector3 get_tangential_velocity();

	btVector3 get_relative_position();

	glm::dvec3 transform_axis(glm::dvec3 axis);

	// Returns the Z axis transformed
	glm::dvec3 get_forward();

	// Returns the Y axis transformed
	glm::dvec3 get_up();

	// Returns the X axis transformed
	glm::dvec3 get_right();

	glm::dmat4 get_graphics_matrix();

	void set_dirty();

	// The piece OWNS the link, which can be null
	std::unique_ptr<Link> link;
	// The point in our collider where the link originates
	glm::dvec3 link_from;
	// The point in the other collider where the link arrives
	glm::dvec3 link_to;
	// What axis should be the link's forward? In part relative coordinates
	glm::dquat link_rot;
	// Only present on stacked parts, what attachment of the target part is used for the link
	std::string to_attachment;
	// Should be present on all parts
	std::string from_attachment;

	std::unordered_map<std::string, Marker> markers;
	// Attachments don't have any significance in physics, but they are important
	// for editing vehicles, so we store them, and a bool marking whether they are used 
	// They are externally managed by the editor (or by wathever changes the vehicle in flight)
	std::vector<std::pair<PieceAttachment, bool>> attachments;	

	Marker& get_marker(const std::string& marker_name);

	glm::dvec3 get_marker_position(const std::string& marker_name);
	glm::dquat get_marker_rotation(const std::string& marker_name);
	glm::dmat4 get_marker_transform(const std::string& marker_name);
	glm::dvec3 get_marker_forward(const std::string& marker_name);

	// Transforms a point into rigidbody relative coordinates (But not rotated!)
	// Useful for applying forces
	glm::dvec3 transform_point_to_rigidbody(glm::dvec3 p);

	Piece(Part* in_part, std::string piece_name);
	~Piece();

	// Pieces cannot be copied, nor moved
	Piece(Piece&&) = default;
	Piece& operator=(Piece&&) = default;

};

