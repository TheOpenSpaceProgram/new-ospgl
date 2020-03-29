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

	WorldState root_state;
	btTransform root_transform;

public:

	void set_world_state(WorldState n_state);

	PackedVehicle(Vehicle* v);
	btTransform get_root_transform(){ return root_transform; }
	WorldState get_root_state(){ return root_state; }

};
