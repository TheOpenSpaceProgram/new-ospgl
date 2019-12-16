#include "Vehicle.h"
#include <glm/glm.hpp>
#include "../util/DebugDrawer.h"
#include "../physics/glm/BulletGlmCompat.h"

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
	st.transform = piece->get_global_transform();
	st.linear = piece->get_linear_velocity();
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
	std::unordered_map<Piece*, PieceState>& states_at_start, btDynamicsWorld* world)
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

			// TODO: Remove link from part
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

		world->addRigidBody(rigid_body);

		btVector3 total_impulse = btVector3(0.0, 0.0, 0.0);
		btVector3 total_angvel = btVector3(0.0, 0.0, 0.0);

		for (Piece* p : wg.first)
		{
			n_group->pieces.push_back(p);
			p->in_group = n_group;
			p->rigid_body = rigid_body;
			p->motion_state = motion_state;
			p->welded_tform = principal_inverse * p->welded_tform;

			rigid_body->applyCentralImpulse(states_at_start[p].linear * p->mass);
			total_angvel += states_at_start[p].angular;
		}

		// TODO: Is this stuff correct?
		total_angvel /= (btScalar)wg.first.size();

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

	world->addRigidBody(rigid_body);

	piece->rigid_body = rigid_body;
	piece->motion_state = motion_state;

	// Apply old impulses
	rigid_body->applyImpulse(states_at_start[piece].linear * piece->mass, piece->get_relative_position());
	rigid_body->setAngularVelocity(states_at_start[piece].angular);
}

std::vector<Vehicle*> Vehicle::update()
{
	std::vector<Vehicle*> n_vehicles;

	if (dirty)
	{
		n_vehicles = handle_separation();

		sort(); //< Not sure if needed

		build_physics();
	}

	return n_vehicles;
}

void Vehicle::build_physics()
{	

	// We need to create shared colliders for all welded 
	// groups, and individual colliders for every other piece
	// The bool is used later on to check if the group was already present
	
	std::vector<WeldedGroupCreation> welded_groups;

	// We need this as we will remove rigidbodies, and with them,
	// physics information. 
	std::unordered_map<Piece*, PieceState> states_at_start;

	for (Piece* piece : all_pieces)
	{
		piece->in_vehicle = this;
		states_at_start[piece] = obtain_piece_state(piece);

		add_to_welded_groups(welded_groups, piece);
	}

	std::vector<Piece*> single_pieces = extract_single_pieces(welded_groups);
	this->single_pieces = single_pieces;

	remove_outdated_welded_groups(welded, welded_groups, world);

	for (WeldedGroupCreation& wg : welded_groups)
	{
		create_new_welded_group(welded, wg, states_at_start, world);
	}

	for (Piece* piece : single_pieces)
	{
		if (piece->rigid_body == nullptr)
		{
			create_piece_physics(piece, states_at_start, world);
		}
	}


	// TODO: Create links between non-welded parts

}

void Vehicle::add_piece(Piece* piece, btTransform pos)
{

	all_pieces.push_back(piece);
	
	add_piece_physics(piece, pos, world);

	piece->in_vehicle = this;

}

std::vector<Vehicle*> Vehicle::handle_separation()
{
	std::vector<Vehicle*> n_vehicles;

	// Find all pieces that can't reach root, and create a new vehicle from them
	// Assumes the vehicle was sorted before the part separated!
	// (Don't sort with a part separated)

	std::vector<std::vector<Piece*>> n_pieces;

	for (auto it = all_pieces.begin(); it != all_pieces.end(); )
	{
		Piece* p = *it;

		if (p == root)
		{
			it++;
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
				}
			}
		}

		if (p->attached_to == nullptr)
		{
			n_pieces.push_back(std::vector<Piece*>());
			n_pieces[n_pieces.size() - 1].push_back(p);

			it = all_pieces.erase(it);
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
						it = all_pieces.erase(it);
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

	for (auto& n_vessel_pieces : n_pieces)
	{
		Vehicle* n_vehicle = new Vehicle(world);

		n_vehicle->all_pieces = n_vessel_pieces;
		n_vehicle->root = n_vessel_pieces[0];

		n_vehicle->sort();
		n_vehicle->build_physics();

		n_vehicles.push_back(n_vehicle);
	}

	return n_vehicles;
}

void Vehicle::draw_debug()
{
	for (size_t i = 0; i < all_pieces.size(); i++)
	{
		glm::vec3 color = glm::vec3(0.7, 0.7, 0.7);

		Piece* p = all_pieces[i];
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

void Vehicle::set_position(btVector3 pos)
{
	for (WeldedGroup* g : welded)
	{
		g->rigid_body->getWorldTransform().setOrigin(pos);
	}

	for (Piece* p : single_pieces)
	{
		p->rigid_body->getWorldTransform().setOrigin(pos);
	}
}

void Vehicle::set_linear_velocity(btVector3 vel)
{
	for (WeldedGroup* g : welded)
	{
		g->rigid_body->setLinearVelocity(vel);
	}

	for (Piece* p : single_pieces)
	{
		p->rigid_body->setLinearVelocity(vel);
	}
}

void Vehicle::sort()
{
	std::unordered_set<Piece*> open;

	open.insert(root);

	std::vector<Piece*> sorted;
	sorted.push_back(root);

	while (!open.empty())
	{
		std::unordered_set<Piece*> new_open;

		for (Piece* o : open)
		{

			for (Piece* p : all_pieces)
			{
				if (p->attached_to == o)
				{
					new_open.insert(p);
					sorted.push_back(p);
				}
			}
		}

		open = new_open;
		
	}

	logger->check(sorted.size() == all_pieces.size(), "Vehicle was sorted while some pieces were not attached!");

	this->all_pieces = sorted;
}

Vehicle::Vehicle(btDynamicsWorld* world)
{
	this->world = world;
}


Vehicle::~Vehicle()
{
}
