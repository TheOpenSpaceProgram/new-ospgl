#pragma once
#include "../part/Machine.h"
#include <unordered_map>
#include <queue>

class Vehicle;

struct PipeJunction;

// Pipes are special and are not a machine, same with BASIC junctions
struct Pipe
{
	// Orientation is only important for the value of flow
	FluidPort *a, *b;

	// For pipe serialization as fluid ports are initialized a bit late, we hold these pointers and
	// resolve them later
	PlumbingMachine *amachine, *bmachine;
	std::string aport, bport;


	float surface;

	// Real-time updated, values greater than 0 mean going from a to b, or going into the junction
	// Remember to multiply by dt before updating!
	float flow;

	// Editor only but serialized
	// They are helpful if your pipes get cluttered up and can be added as the
	// pipe is built
	std::vector<glm::ivec2> waypoints;
	void invert();

	Pipe();
};


// Do not hold pointers to pipes for long as they may be modified, use array indices instead
class VehiclePlumbing
{
private:

	// A flow path connects two ports
	struct FlowPath
	{
		// The pressure difference, considering flow machines, between start and end ports
		float delta_P;
		std::vector<size_t> path;
		bool backwards;
	};

	// For storing the IDs
	friend class VehicleLoader;
	friend class VehicleSaver;

	Vehicle* veh;

	void find_all_possible_paths(std::vector<FlowPath>& fws);
	// Starts assuming start.a contains the true machine!
	void find_all_possible_paths_from(std::vector<FlowPath>& fws, size_t start, bool backwards);
	void calculate_delta_p(FlowPath& fpath, bool backwards);
	// Returns the indices of the paths that are forced
	std::vector<size_t> find_forced_paths(std::vector<FlowPath>& fws);
	bool remove_paths_not_compatible_with_forced(std::vector<FlowPath>& fws);
	void reduce_to_forced_paths(std::vector<FlowPath>& fws);
	// Two paths are considered to be compatible if they don't have a point in which they have opposite direction
	bool are_paths_compatible(const FlowPath& a, const FlowPath& b);
	void execute_flows(float dt, std::vector<FlowPath>& flows);

public:

	// These functions check and area of the plumbing grid for machines
	std::vector<PlumbingMachine*> grid_aabb_check(glm::vec2 start, glm::vec2 end, bool expand = false)
		{ return grid_aabb_check(start, end, {}, expand); }
	std::vector<PlumbingMachine*> grid_aabb_check(glm::vec2 start, glm::vec2 end, const std::vector<PlumbingMachine*>& ignore,
												  bool expand = false);

	// Combines virtual and real machines
	std::vector<PlumbingMachine*> get_all_elements();
	std::vector<Machine*> get_all_true_ports();

	std::vector<Pipe> pipes;
	// Includes stuff such as virtual pumps and junctions
	std::vector<PlumbingMachine*> plumbing_machines;

	void update_pipes(float dt, Vehicle* in_vehicle);
	// Called when adding new parts, or merging vehicles (etc...)
	glm::ivec2 find_free_space(glm::ivec2 size);
	// Returns (0, 0) if none of the machines have plumbing
	glm::ivec2 get_plumbing_size_of(Part* p);

	// Returns an AABB (pos, size) with the bounds of used plumbing area by machines
	glm::ivec4 get_plumbing_bounds();

	// Creates a pipe and returns its id in the array (for convenience, it's pipes.size() - 1!)
	int create_pipe();
	// Doesn't invalidate pointers as pipe_junctions are in the heap
	PipeJunction* create_pipe_junction();

	// Finds the pipe that is connected to given port, wether at a or b
	// Returs pipe index in the array or negative if not found
	int find_pipe_connected_to(FluidPort* port);

	explicit VehiclePlumbing(Vehicle* in_vehicle);

	// Really creates the plumbing connections after loading a vehicle
	void init();

};