#include "PackedVehicle.h"
#include "Vehicle.h"

PackedVehicle::PackedVehicle(Vehicle* v)
{
	this->vehicle = v;
}


void PackedVehicle::set_world_state(WorldState n_state)
{
	root_state = n_state;

	// Calculate new root
	root_transform.setOrigin(to_btVector3(root_state.cartesian.pos));
	root_transform.setRotation(to_btQuaternion(root_state.rotation));	
}

void PackedVehicle::calculate_com()
{
	btVector3 acc = btVector3(0, 0, 0);
	double tot_mass = 0.0;
	for(Piece* p : vehicle->all_pieces)
	{
		acc += p->packed_tform.getOrigin() * p->mass;
		tot_mass += p->mass;
	}

	com = acc / tot_mass;
}
