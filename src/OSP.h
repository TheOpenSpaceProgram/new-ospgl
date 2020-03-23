#pragma once
#include <argh.h>
#include <rang.hpp>
#include "util/Logger.h"
#include <cpptoml.h>
#include <memory>
#include "util/SerializeUtil.h"
#include "assets/AssetManager.h"
#include "renderer/Renderer.h"
#include "util/render/TextDrawer.h"
#include "util/InputUtil.h"
#include "lua/LuaCore.h"
#include "util/defines.h"
#include "util/Timer.h"

// Initializes the different subsystems OSP has
class OSP
{
public:

	Timer dtt;
	double dt;
	// Running time of the game, ignoring time-warp and everything like that
	double game_t;

	Renderer* renderer;

	constexpr static const char* OSP_VERSION = "PRE-RELEASE";
	void init(int argc, char** argv);
	void finish();
	
	bool should_loop();
	// Call before update and render
	void start_frame();
	// Call after render
	void finish_frame(double max_dt = 9999999999.9);
};
