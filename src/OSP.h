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

// Initializes the different subsystems OSP has
class OSP
{
public:

	Renderer* renderer;

	constexpr static const char* OSP_VERSION = "PRE-RELEASE";
	void init(int argc, char** argv);

	void finish();
	
};
