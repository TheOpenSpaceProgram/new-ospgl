#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include "../../util/ThreadUtil.h"
#include "PlanetTilePath.h"
#include "PlanetTile.h"
#include "../quadtree/QuadTreePlanet.h"
#include <set>
#include <array>
#include <thread>
#include <sol.hpp>
#include "../../universe/body/config/PlanetConfig.h"
#include <FastNoise/FastNoise.h>

struct PlanetTileThread
{
	sol::state lua_state;
	std::thread* thread;
	FastNoise noise;
};

// The tile server handles storage, creation and removal
// of tiles via a simple interface.
// This is the "master" of tile generation, while
// the "slave" threads will do the weight lifting
class PlanetTileServer
{
private:

	FastNoise noise;

	bool dirty;

	static const size_t WORKER_THREAD_COUNT = 2;

	int depth_for_unload;

	std::array<PlanetTileThread, WORKER_THREAD_COUNT> threads;

	static void thread_func(PlanetTileServer* server, PlanetTileThread* thread);

	void prepare_lua(sol::state& lua_state, FastNoise* noise);

	// Loads default values for the different libraries
	void default_lua(sol::state& lua_state, FastNoise* noise);

	// We keep a little state to find height and so 
	// everybody can query to find stuff about the script
	sol::state lua_state;

	int noise_seed;
	FastNoise::Interp noise_interp;

public:

	bool has_water;

	PlanetConfig* config;

	bool has_errors;

	bool threads_run;

	using TileMap = std::unordered_map<PlanetTilePath, PlanetTile*, PlanetTilePathHasher>;

	// Used for waking up threads efficiently, instead
	// of a wait-loop which uses 100% CPU
	std::mutex condition_mtx;
	std::condition_variable condition_var;

	Atomic<TileMap> tiles;
	// Threads always try to work on the highest priority
	// (ie. lowest detail tile) first
	Atomic<std::multiset<PlanetTilePath, PlanetTilePathLess>> work_list;

	// Tells threads to start loading some new tiles, if neccesary
	// or unloads unused, small enough tiles.
	void update(QuadTreePlanet& planet);

	// Any tile deeper than, or equal to this will be unloaded
	// **when unused**
	// Default value is 0, so all tiles but the root ones 
	// are unloaded the moment they are not needed
	void set_depth_for_unload(int depth);

	void do_imgui();

	bool is_built()
	{
		return work_list.get_unsafe()->size() == 0;
	}
	
	double get_height(glm::dvec3 pos_3d, size_t depth = 1);

	// Make sure you call once a OpenGL context is available
	// as we will create the index buffer here
	PlanetTileServer(const std::string& script, PlanetConfig* config, int seed, int noise_interp, bool has_water);
	~PlanetTileServer();
};


