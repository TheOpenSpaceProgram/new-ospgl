#include "Vehicle.h"



void Vehicle::build_physics()
{	
	// We need to create shared colliders for all welded 
	// groups, and individual colliders for every other piece
	std::vector<std::unordered_set<Piece*>> welded_groups;
	std::vector<Piece*> single_pieces;

	for (Piece* piece : all_pieces)
	{
		if (piece->is_welded)
		{
			bool found = false;
			// Find what welded group to add this to
			for (size_t i = 0; i < welded_groups.size(); i++)
			{
				if (welded_groups[i].count(piece->attached_to) != 0)
				{
					welded_groups[i].insert(piece);
					found = true;
					break;
				}
			}

			if (!found)
			{
				welded_groups.push_back(std::unordered_set<Piece*>());
				welded_groups[welded_groups.size() - 1].insert(piece);
				welded_groups[welded_groups.size() - 1].insert(piece->attached_to);
			}
		}
		else
		{
			single_pieces.push_back(piece);
		}
	}

	// TODO: Remove welded colliders (and rigidbodies) that changed
	for (WeldedGroup& wgroup : welded)
	{
		bool found = false;
		// Find any same group, to be the same, the welded_group
		// must contain every piece in wgroup
		for (size_t j = 0; j < welded_groups.size(); j++)
		{
			int count = 0;
			for (size_t i = 0; i < wgroup.pieces.size(); i++)
			{
				count += welded_groups[j].count(wgroup.pieces[i]) == 0 ? 0 : 1;
			}

			if (count == wgroup.pieces.size())
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			// Remove this group and all its data

		}
	}

	// TODO: Remove single rigidbodies which are now welded

	// Welded groups now contains an entry for every single group of parts welded together
	// Pretty easy to do this now
	for (std::unordered_set<Piece*>& group : welded_groups)
	{
		// TODO: Create a shared collider and rigidbody if it doesn't have one
	}

	for (Piece* piece : single_pieces)
	{
		// TODO: Create a single rigidbody if it doesn't have one
	}

	// TODO: Create physics links between pieces (and welded groups)

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

Vehicle::Vehicle()
{
}


Vehicle::~Vehicle()
{
}
