#pragma once
#include <vector>
#include <string>

class Part;
class Vehicle;

class VehicleMeta
{
private:
	Vehicle* veh;
public:
	// The ID of the group is the index in the array, this is handled
	// nicely by the utility functions
	std::vector<std::string> group_names;
	std::vector<Part*> find_parts_in_group(int64_t group);
	void remove_group(int64_t id);
	void create_group(std::string name);
	std::string get_group_name(int64_t id);

	VehicleMeta(Vehicle* v) : veh(v) {}

};