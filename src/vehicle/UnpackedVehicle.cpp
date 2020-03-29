#include "UnpackedVehicle.h"
#include <glm/glm.hpp>
#include "../util/DebugDrawer.h"
#include "../physics/glm/BulletGlmCompat.h"
#include "Vehicle.h"

using WeldedGroupCreation = std::pair<std::unordered_set<Piece*>, bool>;

struct PieceState
{
	btTransform transform;
	btVector3 linear;
	btVector3 angular;
};

static PieceState obtain_piece_state(Piece* piece)
{
	PieceState st;
	st.transform = piece->get_global_transform(false);
	st.linear = piece->get_linear_velocity(true); // < TODO: Maybe that should be false and not true!
	st.angular = piece->get_angular_velocity();
	return st;
}

static void add_to_welded_groups(std::vector<WeldedGroupCreation>& welded_groups, Piece* piece)
{
	bool found = false;
	// Find what welded group to add this to
	if (piece->welded)
	{
		for (size_t i = 0; i < welded_groups.size(); i++)
		{
			if (welded_groups[i].first.count(piece->attached_to) != 0)
			{
				welded_groups[i].first.insert(piece);
				found = true;
				break;
			}
		}
	}

	if (!found)
	{
		// Create the welded group, add the part, and, if this part is welded, add
		// whatever it's welded to (Note: individual parts get a lone welded group)
		welded_groups.push_back(std::make_pair(std::unordered_set<Piece*>(), false));
		welded_groups[welded_groups.size() - 1].first.insert(piece);
		if (piece->welded)
		{
			welded_groups[welded_groups.size() - 1].first.insert(piece->attached_to);
		}
	}
}

static std::vector<Piece*> extract_single_pieces(std::vector<WeldedGroupCreation>& welded_groups)
{
	std::vector<Piece*> single_pieces;

	// Find single piece (groups of only one piece)
	for (auto it = welded_groups.begin(); it != welded_groups.end();)
	{
		if (it->first.size() == 1)
		{
			Piece* piece = *it->first.begin();

			single_pieces.push_back(piece);
			// Remove welded colliders, if it had any
			if (piece->in_group != nullptr)
			{
				piece->rigid_body = nullptr;
				piece->motion_state = nullptr;
				piece->in_group = nullptr;
				piece->welded_collider_id = -1;
			}

			it = welded_groups.erase(it);
		}
		else
		{
			it++;
		}
	}

	return single_pieces;
}

static void remove_outdated_welded_groups(
	std::vector<WeldedGroup*>& welded, std::vector<WeldedGroupCreation>& welded_groups, btDynamicsWorld* world)
{
	for (auto it = welded.begin(); it != welded.end();)
	{
		WeldedGroup* wgroup = *it;

		bool found = false;
		// Find any same group, to be the same, the welded_group
		// must contain every piece in wgroup, and be the same size
		for (size_t j = 0; j < welded_groups.size(); j++)
		{
			int count = 0;
			for (size_t i = 0; i < wgroup->pieces.size(); i++)
			{
				count += (int)welded_groups[j].first.count(wgroup->pieces[i]);
			}

			if (count == wgroup->pieces.size() && !wgroup->dirty)
			{
				found = true;
				welded_groups[j].second = true;
				break;
			}
		}

		if (!found)
		{
			// We remove the group as it is not present anymore
			world->removeRigidBody(wgroup->rigid_body);

			// Set all group rigidbodies to nullptr!
			for (Piece* p : wgroup->pieces)
			{
				p->rigid_body = nullptr;
			}

			delete wgroup->motion_state;
			delete wgroup->rigid_body;
			delete wgroup;



			it = welded.erase(it);
		}
		else
		{
			it++;
		}
	}
}

static void create_new_welded_group(
	std::vector<WeldedGroup*>& welded, WeldedGroupCreation& wg, 
	std::unordered_map<Piece*, PieceState>& states_at_start, btDynamicsWorld* world,
	std::vector<Piece*>& all_pieces)
{
	if (wg.second == false)
	{
		// Create a new WeldedGroup
		WeldedGroup* n_group = new WeldedGroup();
		n_group->pieces.reserve(wg.first.size());

		// Create collider
		btCompoundShape temp_collider = btCompoundShape();
		btCompoundShape* collider = new btCompoundShape();

		btTransform identity;
		identity.setIdentity();

		std::vector<btScalar> masses;
		btTransform principal;
		principal.setIdentity();

		double tot_mass = 0.0;

		for (Piece* p : wg.first)
		{
			p->welded_tform = states_at_start[p].transform;
			temp_collider.addChildShape(p->welded_tform, p->collider);
			masses.push_back(p->mass);
			tot_mass += p->mass;

			p->welded_collider_id = temp_collider.getNumChildShapes() - 1;
		}

		// Create rigidbody
		btVector3 local_inertia;

		temp_collider.calculatePrincipalAxisTransform(masses.data(), principal, local_inertia);

		btTransform principal_inverse = principal.inverse();

		for (int i = 0; i < temp_collider.getNumChildShapes(); i++)
		{
			collider->addChildShape(principal_inverse * temp_collider.getChildTransform(i),
				temp_collider.getChildShape(i));
		}


		//collider->calculateLocalInertia(tot_mass, local_inertia);

		btMotionState* motion_state = new btDefaultMotionState(principal);
		btRigidBody::btRigidBodyConstructionInfo info(tot_mass, motion_state, collider, local_inertia);
		btRigidBody* rigid_body = new btRigidBody(info);

		rigid_body->setActivationState(DISABLE_DEACTIVATION);

		// TODO: Think, maybe we can do the average of all parts? Maybe using default values is good
		rigid_body->setFriction(PIECE_DEFAULT_FRICTION);
		rigid_body->setRestitution(PIECE_DEFAULT_RESTITUTION);

		world->addRigidBody(rigid_body);

		btVector3 total_impulse = btVector3(0.0, 0.0, 0.0);
		btVector3 total_angvel = btVector3(0.0, 0.0, 0.0);

		for (Piece* p : wg.first)
		{
			n_group->pieces.push_back(p);
			p->in_group = n_group;

			if (p->rigid_body != nullptr)
			{
				// We must be careful here, we may be removing an already welded rigidbody!
				for (Piece* sp : wg.first)
				{
					if (sp->rigid_body == p->rigid_body && sp != p)
					{
						sp->rigid_body = nullptr;
					}
				}

				world->removeRigidBody(p->rigid_body);
				delete p->rigid_body;
			}

			p->rigid_body = rigid_body;
			p->motion_state = motion_state;
			p->welded_tform = principal_inverse * p->welded_tform;

			rigid_body->applyCentralImpulse(states_at_start[p].linear * p->mass);
			total_angvel += states_at_start[p].angular;
			//total_impulse += states_at_start[p].linear;
		}

		// TODO: Is this stuff correct?
		total_angvel /= (btScalar)wg.first.size();
	//	total_impulse /= (btScalar)wg.first.size();

		//rigid_body->applyCentralImpulse(total_impulse * tot_mass);
		rigid_body->setAngularVelocity(total_angvel);

		n_group->rigid_body = rigid_body;
		n_group->motion_state = motion_state;

		welded.push_back(n_group);
	}
}

static void add_piece_physics(Piece* piece, btTransform tform, btDynamicsWorld* world)
{
	btVector3 local_inertia;
	piece->collider->calculateLocalInertia(piece->mass, local_inertia);

	btMotionState* motion_state = new btDefaultMotionState(tform);
	btRigidBody::btRigidBodyConstructionInfo info(piece->mass, motion_state, piece->collider, local_inertia);
	btRigidBody* rigid_body = new btRigidBody(info);

	rigid_body->setActivationState(DISABLE_DEACTIVATION);

	rigid_body->setFriction(piece->friction);
	rigid_body->setRestitution(piece->restitution);

	world->addRigidBody(rigid_body);

	piece->rigid_body = rigid_body;
	piece->motion_state = motion_state;
}

static void create_piece_physics(Piece* piece, std::unordered_map<Piece*, PieceState>& states_at_start, btDynamicsWorld* world)
{
	// We must be a lone piece if we haven't been touched before
	btVector3 local_inertia;
	piece->collider->calculateLocalInertia(piece->mass, local_inertia);

	btMotionState* motion_state = new btDefaultMotionState(states_at_start[piece].transform);
	btRigidBody::btRigidBodyConstructionInfo info(piece->mass, motion_state, piece->collider, local_inertia);
	btRigidBody* rigid_body = new btRigidBody(info);

	rigid_body->setActivationState(DISABLE_DEACTIVATION);

	rigid_body->setFriction(piece->friction);
	rigid_body->setRestitution(piece->restitution);
	rigid_body->setWorldTransform(states_at_start[piece].transform);

	world->addRigidBody(rigid_body);

	piece->rigid_body = rigid_body;
	piece->motion_state = motion_state;

	// Apply old impulses
	rigid_body->applyCentralImpulse(states_at_start[piece].linear * piece->mass);
	rigid_body->setAngularVelocity(states_at_start[piece].angular);
}

std::vector<Vehicle*> UnpackedVehicle::update()
{
	std::vector<Vehicle*> n_vehicles;
	// Check for any broken links, they instantly set the dirty flags,
	// but they cannot set it themselves
	for (Piece* p : vehicle->all_pieces)
	{
		if (p->link != nullptr && p->attached_to != nullptr)
		{
			if (p->link->is_broken())
			{
				dirty = true;
				break;
			}
		}
	}

	if (dirty)
	{
		n_vehicles = handle_separation();

		vehicle->sort(); //< Not sure if needed

		build_physics();

		for (Piece* p : vehicle->all_pieces)
		{
			if (p->link != nullptr && p->attached_to != nullptr)
			{
				p->link->set_breaking_enabled(breaking_enabled);
			}
		}

		dirty = false;

	}

	return n_vehicles;
}

void UnpackedVehicle::build_physics()
{	
	// Remove all old links
	for (Piece* piece : vehicle->all_pieces)
	{
		if (piece->link != nullptr)
		{
			piece->link->deactivate();
		}
	}

	// We need to create shared colliders for all welded 
	// groups, and individual colliders for every other piece
	// The bool is used later on to check if the group was already present
	
	std::vector<WeldedGroupCreation> welded_groups;

	// We need this as we will remove rigidbodies, and with them,
	// physics information. 
	std::unordered_map<Piece*, PieceState> states_at_start;

	for (Piece* piece : vehicle->all_pieces)
	{
		piece->in_vehicle = vehicle;
		states_at_start[piece] = obtain_piece_state(piece);
	
		add_to_welded_groups(welded_groups, piece);
	}

	std::vector<Piece*> single_pieces = extract_single_pieces(welded_groups);
	this->single_pieces = single_pieces;

	remove_outdated_welded_groups(welded, welded_groups, world);

	for (WeldedGroupCreation& wg : welded_groups)
	{
		create_new_welded_group(welded, wg, states_at_start, world, vehicle->all_pieces);
	}

	for (Piece* piece : single_pieces)
	{
		if (piece->rigid_body == nullptr)
		{
			create_piece_physics(piece, states_at_start, world);
		}
	}

	for (Piece* piece : vehicle->all_pieces)
	{
		// piece->attached_to cannot have null rigidbody as it will have already been built
		// in the previous loop
		if (piece->attached_to != nullptr && piece->link != nullptr && !piece->welded)
		{
			btTransform from_tform = btTransform::getIdentity();
			btTransform to_tform = btTransform::getIdentity();

			from_tform.setOrigin(to_btVector3(piece->link_from));
			from_tform.setRotation(to_btQuaternion(piece->link_rot));
			to_tform.setOrigin(to_btVector3(piece->link_to));

			btTransform real_from = piece->get_local_transform() * from_tform;
			btTransform real_to = piece->attached_to->get_local_transform() * to_tform;
			piece->link->activate(piece->rigid_body, real_from, piece->attached_to->rigid_body, real_to, world);
		}
	}

	

}

void UnpackedVehicle::add_piece(Piece* piece, btTransform pos)
{

	vehicle->all_pieces.push_back(piece);
	
	add_piece_physics(piece, pos, world);

	piece->in_vehicle = vehicle;

}

std::vector<Vehicle*> UnpackedVehicle::handle_separation()
{
	std::vector<Vehicle*> n_vehicles;

	std::unordered_set<WeldedGroup*> wgroups;

	// First pass to remove any broken links
	for (auto it = vehicle->all_pieces.begin(); it != vehicle->all_pieces.end(); it++)
	{
		Piece* p = *it;

		if (p == vehicle->root)
		{
			continue;
		}

		// Handle broken links
		if (p->welded == false)
		{
			if (p->link == nullptr)
			{
				p->attached_to = nullptr;
			}
			else
			{
				if (p->link->is_broken())
				{
					p->attached_to = nullptr;
					p->link->deactivate();
				}
			}
		}
	}

	// Find all pieces that can't reach root, and create a new vehicle from them
	// Assumes the vehicle was sorted before the part separated!
	// (Don't sort with a part separated)

	std::vector<std::vector<Piece*>> n_pieces;

	for (auto it = vehicle->all_pieces.begin(); it != vehicle->all_pieces.end(); )
	{
		Piece* p = *it;

		if (p == vehicle->root)
		{
			it++;
			continue;
		}


		if (p->attached_to == nullptr)
		{
			n_pieces.push_back(std::vector<Piece*>());
			n_pieces[n_pieces.size() - 1].push_back(p);

			it = vehicle->all_pieces.erase(it);
		}
		else
		{
			bool removed = false;

			for (auto& list : n_pieces)
			{
				for (auto sub_p : list)
				{
					if (p->attached_to == sub_p)
					{
						list.push_back(p);
						it = vehicle->all_pieces.erase(it);
						removed = true;

						// Lord, I'm sorry, but this is the cleanest option
						goto out;
					}
				}
			}

			out:

			if (!removed)
			{
				it++;
			}
		}
	}
	
	// Find welded groups to transfer

	for (auto& n_vessel_pieces : n_pieces)
	{
		for (WeldedGroup* w : welded)
		{
			for (size_t i = 0; i < n_vessel_pieces.size(); i++)
			{
				if (w->rigid_body == n_vessel_pieces[i]->rigid_body)
				{
					wgroups.insert(w);
				}
			}
		}
	}


	// Here we remove the welded groups from original vessel
	for (WeldedGroup* w : wgroups)
	{
		for (size_t i = 0; i < welded.size(); i++)
		{
			if (welded[i] == w)
			{
				welded.erase(welded.begin() + i);
				break;
			}
		}
	}

	for (auto& n_vessel_pieces : n_pieces)
	{

		Vehicle* n_vehicle = new Vehicle();
		n_vehicle->unpacked_veh.set_world(world);
	
		n_vehicle->all_pieces = n_vessel_pieces;
		n_vehicle->root = n_vessel_pieces[0];

		for (WeldedGroup* w : wgroups)
		{
			for (size_t i = 0; i < w->pieces.size(); i++)
			{
				for (size_t j = 0; j < n_vessel_pieces.size(); j++)
				{
					if (n_vessel_pieces[j] == w->pieces[i])
					{
						// Transfer the welded group
						n_vehicle->unpacked_veh.welded.push_back(w);
						i = w->pieces.size() + 1;
						break;
					}
				}

			}

		}

		n_vehicle->packed = false;
		n_vehicle->sort();
		n_vehicle->unpacked_veh.build_physics();

		logger->info("Separated new vehicle");
		n_vehicles.push_back(n_vehicle);
	}



	return n_vehicles;
}


void UnpackedVehicle::set_position(glm::dvec3 pos)
{
	btVector3 bt = to_btVector3(pos);

	btVector3 root_pos = vehicle->root->get_global_transform().getOrigin();
	for (WeldedGroup* g : welded)
	{
		btVector3 off = g->rigid_body->getWorldTransform().getOrigin() - root_pos;
		g->rigid_body->getWorldTransform().setOrigin(bt + off);
	}

	for (Piece* p : single_pieces)
	{
		btVector3 off = p->get_global_transform().getOrigin() - root_pos;
		p->rigid_body->getWorldTransform().setOrigin(bt + off);
	}
}

void UnpackedVehicle::set_linear_velocity(glm::dvec3 vel)
{
	btVector3 bt = to_btVector3(vel);

	btVector3 root_vel = vehicle->root->rigid_body->getLinearVelocity();

	for (WeldedGroup* g : welded)
	{
		btVector3 off = g->rigid_body->getLinearVelocity() - root_vel;
		g->rigid_body->setLinearVelocity(bt + off);
	}

	for (Piece* p : single_pieces)
	{
		btVector3 off = p->rigid_body->getLinearVelocity() - root_vel;
		p->rigid_body->setLinearVelocity(bt + off);
	}
}

void UnpackedVehicle::set_breaking_enabled(bool value)
{
	dirty = true;
	this->breaking_enabled = value;
}



void UnpackedVehicle::draw_debug()
{
	for (size_t i = 0; i < vehicle->all_pieces.size(); i++)
	{
		glm::vec3 color = glm::vec3(0.7, 0.7, 0.7);

		Piece* p = vehicle->all_pieces[i];
		Piece* link = p->attached_to;

		if (link == nullptr)
		{
			color = glm::vec3(1.0, 0.7, 1.0);
		}

		glm::dvec3 ppos = to_dvec3(p->get_global_transform().getOrigin());

		debug_drawer->add_point(ppos, color);

		if (link != nullptr)
		{
			glm::dvec3 dpos = to_dvec3(link->get_global_transform().getOrigin());

			if (p->welded)
			{
				debug_drawer->add_line(ppos, dpos, glm::dvec3(0.8, 0.8, 0.8));
			}
			else
			{
				debug_drawer->add_line(ppos, dpos, glm::dvec3(0.8, 0.3, 0.3));
			}
		}
	}
}

void UnpackedVehicle::deactivate()
{
	for(WeldedGroup* group : welded)
	{
		world->removeRigidBody(group->rigid_body);
		delete group->rigid_body;
		delete group->motion_state;
		delete group;
	}

	for(Piece* p : single_pieces)
	{
		world->removeRigidBody(p->rigid_body);
		delete p->rigid_body;
		delete p->motion_state;
	}

	single_pieces.clear();
	welded.clear();	
}

glm::dvec3 UnpackedVehicle::get_center_of_mass()
{
	double tot_mass = 0.0;
	glm::dvec3 out = glm::dvec3(0.0, 0.0, 0.0);
	for(Piece* p : vehicle->all_pieces)
	{
		tot_mass += p->mass;
		out += to_dvec3(p->get_global_transform().getOrigin()) * p->mass;
	}

	out /= tot_mass;

	return out;
}

void UnpackedVehicle::activate()
{
	vehicle->packed = true;
	// sort() 				// TODO: Sorting may not be neccesary here as pieces won't change while packed
	build_physics();
	vehicle->packed = false;
}

UnpackedVehicle::UnpackedVehicle(Vehicle* v)
{
	this->vehicle = v;
}
