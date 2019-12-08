#include "Vehicle.h"
#include <glm/glm.hpp>
#include "../util/DebugDrawer.h"
#include "../physics/glm/BulletGlmCompat.h"

void Vehicle::build_physics(btDynamicsWorld* world)
{	
	using WeldedGroupCreation = std::pair<std::unordered_set<Piece*>, bool>;

	// We need to create shared colliders for all welded 
	// groups, and individual colliders for every other piece
	// The bool is used later on to check if the group was already present
	std::vector<WeldedGroupCreation> welded_groups;
	std::vector<Piece*> single_pieces;

	for (Piece* piece : all_pieces)
	{
		if (piece->welded)
		{
			bool found = false;
			// Find what welded group to add this to
			for (size_t i = 0; i < welded_groups.size(); i++)
			{
				if (welded_groups[i].first.count(piece->attached_to) != 0)
				{
					welded_groups[i].first.insert(piece);
					found = true;
					break;
				}
			}

			if (!found)
			{
				welded_groups.push_back(std::make_pair(std::unordered_set<Piece*>(), false));
				welded_groups[welded_groups.size() - 1].first.insert(piece);
				welded_groups[welded_groups.size() - 1].first.insert(piece->attached_to);
			}

			// Remove single collider
			if (piece->in_group == nullptr)
			{
				if (piece->rigid_body != nullptr)
				{
					world->removeRigidBody(piece->rigid_body);
					delete piece->rigid_body;
					delete piece->motion_state;

				}
			}
		}
		else
		{
			single_pieces.push_back(piece);
			// Remove welded colliders, if it had any
			if (piece->in_group != nullptr)
			{
				piece->rigid_body = nullptr;
				piece->motion_state = nullptr;
				piece->in_group = nullptr;
			}

		}
	}

	// TODO: Remove welded groups that changed
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
				count += welded_groups[j].first.count(wgroup->pieces[i]);
			}

			if (count == wgroup->pieces.size())
			{
				found = true;
				welded_groups[j].second = true;
				break;
			}
		}

		if (!found)
		{
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


	for (WeldedGroupCreation& wg : welded_groups)
	{
		if (wg.second == false)
		{
			// Create a new WeldedGroup
			WeldedGroup* n_group = new WeldedGroup();
			n_group->pieces.reserve(wg.first.size());

			// Create collider
			btCompoundShape* temp_collider = new btCompoundShape();
			btCompoundShape* collider = new btCompoundShape();

			btTransform identity;
			identity.setIdentity();

			std::vector<btScalar> masses;
			btTransform principal;
			principal.setIdentity();

			double tot_mass = 0.0;

			for (Piece* p : wg.first)
			{
				p->welded_tform = p->get_current_position();
				temp_collider->addChildShape(p->welded_tform, p->collider);
				masses.push_back(p->mass);
				tot_mass += p->mass;
			}

			// Create rigidbody
			btVector3 local_inertia;

			temp_collider->calculatePrincipalAxisTransform(masses.data(), principal, local_inertia);

			btTransform principal_inverse = principal.inverse();

			for (int i = 0; i < temp_collider->getNumChildShapes(); i++)
			{
				// Note: If principal_inverse is rightmost term, it breaks
				// TODO: Investigate if this is an error in bullet, as I copied the code from bullet examples
				collider->addChildShape(principal_inverse * temp_collider->getChildTransform(i),
					temp_collider->getChildShape(i));
			}

			delete temp_collider;

			//collider->calculateLocalInertia(tot_mass, local_inertia);

			btMotionState* motion_state = new btDefaultMotionState(principal);
			btRigidBody::btRigidBodyConstructionInfo info(tot_mass, motion_state, collider, local_inertia);
			btRigidBody* rigid_body = new btRigidBody(info);

			world->addRigidBody(rigid_body);

			for (Piece* p : wg.first)
			{
				n_group->pieces.push_back(p);
				p->in_group = n_group;
				p->rigid_body = rigid_body;
				p->motion_state = motion_state;
				p->welded_tform = principal_inverse * p->welded_tform;
			}

			n_group->rigid_body = rigid_body;
			n_group->motion_state = motion_state;

			welded.push_back(n_group);
		}
	}

	for (Piece* piece : single_pieces)
	{
		if (piece->rigid_body == nullptr)
		{
			// We must be a lone piece if we haven't been touched before
			btVector3 local_inertia;
			piece->collider->calculateLocalInertia(piece->mass, local_inertia);

			btMotionState* motion_state = new btDefaultMotionState(piece->position);
			btRigidBody::btRigidBodyConstructionInfo info(piece->mass, motion_state, piece->collider, local_inertia);
			btRigidBody* rigid_body = new btRigidBody(info);

			world->addRigidBody(rigid_body);

			piece->rigid_body = rigid_body;
			piece->motion_state = motion_state;
		}
	}

}

std::vector<Vehicle*> Vehicle::handle_separation()
{
	std::vector<Vehicle*> n_vehicles;

	// Index 0 is always new root part
	std::vector<std::vector<Piece*>> separated_groups;
	for (Piece* piece : all_pieces)
	{
		// Root is always separated ;)
		if (piece != root)
		{
			separated_groups.push_back(std::vector<Piece*>());
			separated_groups[separated_groups.size() - 1].push_back(piece);
		}
	}

	// Find all parts connected to any part in the separated group

	bool done = false;

	while (!done)
	{
		int added = 0;

		for (Piece* piece : all_pieces)
		{

			for (std::vector<Piece*>& group : separated_groups)
			{
				for (Piece* p : group)
				{
					if (piece->attached_to == p)
					{
						group.push_back(piece);
						added++;
					}
				}
			}

		}

		if (added == 0)
		{
			done = true;
		}
	}

	// Remove from our vehicle
	for (std::vector<Piece*>& group : separated_groups)
	{
		for (Piece* p : group)
		{
			for (size_t i = 0; i < all_pieces.size(); i++)
			{
				if (all_pieces[i] == p)
				{
					all_pieces.erase(all_pieces.begin() + i);
				}
			}
		}
	}

	// Add to new vehicles
	for (size_t i = 0; i < separated_groups.size(); i++)
	{
		Vehicle* n_vehicle = new Vehicle();

		for (size_t j = 0; j < separated_groups[i].size(); j++)
		{
			n_vehicle->all_pieces.push_back(separated_groups[i][j]);
		}
		
		n_vehicle->root = separated_groups[i][0];

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

		glm::dvec3 ppos = to_dvec3(p->get_current_position().getOrigin());

		debug_drawer->add_point(ppos, color);
		
		if (link != nullptr)
		{
			glm::dvec3 dpos = to_dvec3(link->get_current_position().getOrigin());

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

Vehicle::Vehicle()
{
}


Vehicle::~Vehicle()
{
}
