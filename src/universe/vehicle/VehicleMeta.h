#pragma once
#include <vector>
#include <string>
#include <memory>
#include "game/scenes/flight/InputContext.h"
#include "SymmetryMode.h"

class Part;
class Machine;
class Vehicle;

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
	std::vector<SymmetryMode*> symmetry_modes;

	// Auto-wiring

	VehicleMeta(Vehicle* v) : veh(v) {
		controlled_part = -1;
		controlled_machine = "";
	}

	// Obtains all symmetric copies of the piece, including nested symmetry groups
	// Optionally includes the piece p as the first vector element
	// If p is in no symmetry group, it's not included!
	std::vector<SymmetryInstance> find_symmetry_instances(Piece* p, bool include_p);
	// Returns array of indices into the array
	std::vector<size_t> find_symmetry_groups_containing(Piece* p);


};