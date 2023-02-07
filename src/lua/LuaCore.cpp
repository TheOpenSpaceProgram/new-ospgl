#include "LuaCore.h"
#include <assets/AssetManager.h>

#include "libs/LuaLogger.h"
#include "libs/LuaGlm.h"
#include "libs/LuaNoise.h"
#include "libs/LuaAssets.h"
#include "libs/LuaBullet.h"
#include "libs/LuaToml.h"
#include "libs/LuaVehicle.h"
#include "libs/LuaDebugDrawer.h"
#include "libs/LuaUniverse.h"
#include "libs/LuaFlightInput.h"
#include "libs/LuaGameDatabase.h"
#include "libs/LuaNanoVG.h"
#include "libs/LuaPlumbing.h"
#include "libs/LuaGUI.h"
#include "libs/LuaImGUI.h"
#include "libs/LuaSceneLib.h"
#include "libs/LuaRenderer.h"
#include "libs/LuaModel.h"
#include "libs/LuaInput.h"
#include "libs/LuaOrbit.h"

// Used for setting up "osp" usertype
#include "renderer/Renderer.h"
#include "audio/AudioEngine.h"
#include "universe/Universe.h"
#include "game/database/GameDatabase.h"


LuaCore* lua_core;
// This function is extremely hacky and contains multiple work-arounds
// for how our lua code is structured
int LoadFileRequire(lua_State* L)
{
	sol::state_view sview = sol::state_view(L);
	std::string path = sol::stack::get<std::string>(L, 1);


	bool was_module = false;
	sol::table module_table = sview.create_table();

	LuaCore::LibraryID id = LuaCore::name_to_id(path);

	if (id != LuaCore::LibraryID::UNKNOWN)
	{
		was_module = true;
		lua_core->load_library(module_table, id);
	}

	if (was_module)
	{
		// TODO: Maybe the index trick is needed here too?
		// We have to do this weird thing to make sure the module's table
		// is returned and not a global table
		// Lua calls this script with only an argument, the module name
		// Note that 'require("wathever")' won't work, you need 'local wathever = require("wathever")'!
		std::string script = "local l = ...; local ret = __loaded_modules[l]; return ret";

		sol::load_result fx = sview.load(script);
		sol::function ffx = fx;
		sol::stack::push(L, ffx);

		sview["__loaded_modules"][path] = module_table;

		return 1;
	}
	else
	{
		// WARNING FOR MODDERS: require creates only one instance, use for libraries!
		auto name = osp->assets->get_filename(path);
		// Enforce local / class files, which may not be "required"
		if(name.rfind("l_", 0) == 0 || name.rfind("c_", 0) == 0)
		{
			logger->fatal("Tried to require {}, which is meant to be included with dofile / loadfile", path);
		}

		std::string spath = osp->assets->resolve_path(path, sview["__pkg"]);

		// The package loader is nothing more than the script itself
		std::string script = osp->assets->load_string_raw(spath);
		sol::load_result fx = sview.load(script, spath);
		sol::function ffx = fx;
		sol::stack::push(L, ffx);
		return 1;

	}
}

LuaCore::LibraryID LuaCore::name_to_id(const std::string & name)
{
	if (name == "logger")
	{
		return LibraryID::LOGGER;
	}
	else if (name == "debug_drawer")
	{
		return LibraryID::DEBUG_DRAWER;
	}
	else if (name == "glm")
	{
		return LibraryID::GLM;
	}
	else if (name == "noise")
	{
		return LibraryID::NOISE;
	}
	else if (name == "assets")
	{
		return LibraryID::ASSETS;
	}
	else if(name == "bullet")
	{
		return LibraryID::BULLET;
	}
	else if(name == "toml")
	{
		return LibraryID::TOML;
	}
	else if(name == "vehicle")
	{
		return LibraryID::VEHICLE;
	}
	else if(name == "universe")
	{
		return LibraryID::UNIVERSE;
	}
	else if(name == "flight_input")
	{
		return LibraryID::FLIGHT_INPUT;
	}
	else if(name == "game_database")
	{
		return LibraryID::GAME_DATABASE;
	}
	else if(name == "nano_vg")
	{
		return LibraryID::NANO_VG;
	}
	else if(name == "plumbing")
	{
		return LibraryID::PLUMBING;
	}
	else if(name == "gui")
	{
		return LibraryID::GUI;
	}
	else if(name == "imgui")
	{
		return LibraryID::IMGUI;
	}
	else if(name == "scene")
	{
		return LibraryID::SCENE;
	}
	else if(name == "renderer")
	{
		return LibraryID::RENDERER;
	}
	else if(name == "model")
	{
		return LibraryID::MODEL;
	}
	else if(name == "input")
	{
		return LibraryID::INPUT;
	}
	else if(name == "orbit")
	{
		return LibraryID::ORBIT;
	}
	else
	{
		return LibraryID::UNKNOWN;
	}
}

void LuaCore::load_library(sol::table& table, LibraryID id)
{
	libraries[id]->load_to(table);
}

void LuaCore::load(sol::state& to, const std::string& pkg)
{
	to.open_libraries(
		sol::lib::base, sol::lib::package, sol::lib::coroutine,
		sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::utf8,
		sol::lib::jit);

	to["print"] = sol::nil;

	to.create_named_table("__loaded_modules");
	load((sol::table&)to.globals(), pkg);

	to.add_package_loader(LoadFileRequire, true);

	// We also create OSP usertype (but don't actually set "osp" to a value, as it may not be available)
	// so that all subsystems can be accessed without having many globals
	to.new_usertype<OSP>("__ut_osp", sol::no_constructor,
		  "renderer", &OSP::renderer,
		  "audio_engine", &OSP::audio_engine,
		  "universe", &OSP::universe,
		  "game_database", &OSP::game_database,
		  "game_dt", sol::readonly(&OSP::game_dt));

}


void LuaCore::load(sol::table& to, const std::string& pkg)
{
	to["__pkg"] = pkg;

	// We override the default dofile and loadfile for package behaviour
	// The file will be passed the environment of the caller (!)
	to["dofile"] = [](const std::string& path, sol::this_environment te, sol::this_state ts)
	{
		sol::environment env = te;
		sol::state_view sv = ts;
		auto name = osp->assets->get_filename(path);
		// Enforce global files, which may not be "dofiled"
		if(name.rfind("g_", 0) == 0)
		{
			logger->fatal("Tried to dofile {}, which is meant to be included with require", path);
		}
		std::string spath = osp->assets->resolve_path(path, env["__pkg"]);

		// We load and execute the file, and return the result
		// We obey the standard and pass errors to the caller with throw
		sol::load_result lresult = sv.load_file(spath);
		if(!lresult.valid())
		{
			sol::error err = lresult;
			throw(err);
		}
		sol::safe_function fnc = lresult;
		sol::set_environment(env, fnc);
		sol::safe_function_result result = fnc();
		if(!result.valid())
		{
			sol::error err = result;
			throw(err);
		}

		return result;
	};

	// The file will be passed the environment of the caller (!)
	to["loadfile"] = [](const std::string& path, sol::this_environment te, sol::this_state ts)
	{
		sol::environment env = te;
		sol::state_view sv = ts;
		auto name = osp->assets->get_filename(path);
		// Enforce global files, which may not be "dofiled"
		if(name.rfind("g_", 0) == 0)
		{
			logger->fatal("Tried to loadfile {}, which is meant to be included with require", path);
		}
		std::string spath = osp->assets->resolve_path(path, env["__pkg"]);

		sol::load_result lresult = sv.load_file(spath);
		if(!lresult.valid())
		{
			std::string err = "Could not find file: " + spath;
			return std::make_tuple((sol::function)sol::nil, err);
		}

		sol::function fnc = lresult;
		sol::set_environment(env, fnc);
		return std::make_tuple(fnc, std::string(""));
	};
}

LuaCore::LuaCore()
{
	libraries[LibraryID::UNKNOWN] = nullptr;
	libraries[LibraryID::LOGGER] = new LuaLogger();
	libraries[LibraryID::DEBUG_DRAWER] = new LuaDebugDrawer();
	libraries[LibraryID::GLM] = new LuaGlm();
	libraries[LibraryID::NOISE] = new LuaNoise();
	libraries[LibraryID::ASSETS] = new LuaAssets();
	libraries[LibraryID::BULLET] = new LuaBullet();
	libraries[LibraryID::TOML] = new LuaToml();
	libraries[LibraryID::VEHICLE] = new LuaVehicle();
	libraries[LibraryID::UNIVERSE] = new LuaUniverse();
	libraries[LibraryID::FLIGHT_INPUT] =  new LuaFlightInput();
	libraries[LibraryID::GAME_DATABASE] = new LuaGameDatabase();
	libraries[LibraryID::NANO_VG] = new LuaNanoVG();
	libraries[LibraryID::PLUMBING] = new LuaPlumbing();
	libraries[LibraryID::GUI] = new LuaGUI();
	libraries[LibraryID::IMGUI] = new LuaImGUI();
	libraries[LibraryID::SCENE] = new LuaSceneLib();
	libraries[LibraryID::RENDERER] = new LuaRenderer();
	libraries[LibraryID::MODEL] = new LuaModel();
	libraries[LibraryID::INPUT] = new LuaInput();
	libraries[LibraryID::ORBIT] = new LuaOrbit();
}

LuaCore::~LuaCore()
{
	for (size_t i = 0; i < (size_t)LibraryID::COUNT; i++)
	{
		delete libraries[i];
	}
}

void create_global_lua_core()
{
	lua_core = new LuaCore();
}

void destroy_global_lua_core()
{
	delete lua_core;
}
