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
class AudioEngine;
class Universe;

// Initializes the different subsystems OSP has
// It's a global class, only one OSP may exist at once (filesystem related)
class OSP
{
public:

	int64_t runtime_uid;

	std::string current_locale;
	Timer dtt;

	// Delta time but is at maximum the physics framerate,
	// use for stuff related to simulation
	double dt{};
	
	// For use with user input unrelated to simulation (cameras and similar)
	double game_dt{};

	AssetManager* assets{};
	Renderer* renderer{};
	AudioEngine* audio_engine{};
	GameState* game_state{};
	GameDatabase* game_database{};
	Universe* universe;

	constexpr static const char* OSP_VERSION = "PRE-RELEASE";
	void init(int argc, char** argv);
	void finish();
	
	bool should_loop();
	// Call before update and render, only does stuff if a renderer is available
	// (Headless doesn't need this function)
	void start_frame();

	void update();

	void render();

	// ASSUMES A GOOD CLEANUP OF PREVIOUS GAMESTATE!
	void launch_gamestate(GameState* st);
	// This basically launches the main menu, which may optionally directly
	// skip to loading of a gamestate
	// ASSUMES A GOOD CLEANUP OF PREVIOUS GAMESTATE (uses launch_gamestate)!
	void launch_menu(const std::string& skip_to_save);
	// Deletes the current game state and launches menu again
	void end_gamestate();

	// Call after render
	void finish_frame();
	uint64_t get_runtime_uid();

	OSP();
};

extern OSP* osp;