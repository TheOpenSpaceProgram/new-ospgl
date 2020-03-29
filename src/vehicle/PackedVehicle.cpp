#include "PackedVehicle.h"


PackedVehicle::PackedVehicle(Vehicle* v)
{

}


void PackedVehicle::set_world_state(WorldState n_state)
{
	root_state = n_state;

	// Calculate new root
	root_transform.setOrigin(to_btVector3(root_state.cartesian.pos));
	root_transform.setRotation(to_btQuaternion(n_state.rotation));	
}
