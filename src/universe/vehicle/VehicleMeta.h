#pragma once
#include <vector>
#include <string>
#include <memory>
#include "game/scenes/flight/InputContext.h"

class Part;
class Machine;
class Vehicle;

// Not neccesarily symmetric, may include non symmetric arrangements
class SymmetryCenter
{
	enum Type
	{
		// Self-explanatory, polygonal radial distribution
		RADIAL_AROUND_PIECE,
		RADIAL_AROUND_AXIS,
		// Self-explanatory,
		PLANAR_AROUND_PIECE,
		PLANAR_AROUND_AXIS,
		// Pieces placed in a series of attachment points
		ATTACHMENT_LIST,
		// Pieces placed in a series of points with a given orientation
		MANY_AXES,
	};
};

class VehicleMeta
{
private:
	Vehicle* veh;
public:

	// Part grouping

	// The ID of the group is the index in the array, this is handled
	// nicely by the utility functions
	std::vector<std::string> group_names;
	std::vector<Part*> find_parts_in_group(int64_t group);
	void remove_group(int64_t id);
	void create_group(std::string name);
	std::string get_group_name(int64_t id);

	// Controlling

	int64_t controlled_part;
	std::string controlled_machine;

	void set_controlled_machine(Machine* m);
	std::shared_ptr<InputContext> get_input_ctx();

	// Symmetry



	// Auto-wiring

	VehicleMeta(Vehicle* v) : veh(v) {
		controlled_part = -1;
		controlled_machine = "";
	}


};