#pragma once
#include <argh.h>
#include <rang.hpp>
#include "util/Logger.h"
#include <cpptoml.h>
#include <memory>
#include "util/SerializeUtil.h"
#include "util/InputUtil.h"
#include "util/defines.h"
#include "util/Timer.h"

class AssetManager;
class Renderer;
class LuaCore;
class GameState;
class GameDatabase;

// Initializes the different subsystems OSP has
// It's a global class, only one OSP may exist at once (filesystem related)
class OSP
{
public:

	std::string current_locale;
	Timer dtt;

	// Delta time but is at maximum the physics framerate,
	// use for stuff related to simulation
	double dt{};
	
	// For use with user input unrelated to simulation (cameras and similar)
	double game_dt{};

	AssetManager* assets{};
	Renderer* renderer{};
	GameState* game_state{};
	GameDatabase* game_database{};

	constexpr static const char* OSP_VERSION = "PRE-RELEASE";
	void init(int argc, char** argv);
	void finish();
	
	bool should_loop();
	// Call before update and render, only does stuff if a renderer is available
	// (Headless doesn't need this function)
	void start_frame();

	void update();

	void render();

	// Call after render
	void finish_frame();

	OSP();
};

extern OSP* osp;