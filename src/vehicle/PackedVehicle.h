#pragma once 

#include "../universe/CartesianState.h"
#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)
#include "../physics/glm/BulletGlmCompat.h"

class Vehicle;

class PackedVehicle
{
private:

	// NOTE: Everything is given relative to root,
	// except the angular velocity which should
	// be assumed to "be" at the COM of the vehicle
	// (To avoid false translation)
	// (Note: The angular velocity of the COM and the root
	// part are the same as the whole thing is a rigidbody,
	// but during calculations the origin of rotation is the
	// COM and not the root part)
	WorldState root_state;
	btTransform root_transform;
	// Center of mass relative to the root part
	btVector3 com;

public:

	Vehicle* vehicle;

	void set_world_state(WorldState n_state);

	PackedVehicle(Vehicle* v);
	btTransform get_root_transform(){ return root_transform; }
	WorldState get_root_state(){ return root_state; }
	btVector3 get_com_root_relative(){ return com; }

	void calculate_com();

};
