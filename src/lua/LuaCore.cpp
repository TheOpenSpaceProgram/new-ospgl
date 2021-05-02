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

LuaCore* lua_core;
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
		// We have to do this weird thing to make sure the module's table
		// is returned and not a global table
		// Lua calls this script with only an argument, the module name
		// Note that 'require("wathever")' won't work, you need 'local wathever = require("wathever")'!
		std::string script = "local l = ...; return __loaded_modules[l];";

		sol::load_result fx = sview.load(script);
		sol::function ffx = fx;
		sol::stack::push(L, ffx);

		sview["__loaded_modules"][path] = module_table;

		return 1;
	}
	else
	{
		// Attempt to load lua file from package

		// We must use lua debug to find the environment of caller here
		lua_Debug info;
		int level = 2;
		int pre_stack_size = lua_gettop(L);
		lua_getstack(L, level, &info);
		lua_getinfo(L, "fnluS", &info);
		sol::function caller(L, -1);
		sol::environment env(sol::env_key, caller);

		std::string pkg = sview["__pkg"].get_or<std::string>("core");

		if(env.valid())
		{
			pkg = env["__pkg"].get_or<std::string>(pkg);
		}


		std::string resolved = osp->assets->resolve_path(path, pkg);
		if (osp->assets->file_exists(resolved))
		{
			std::string file = osp->assets->load_string_raw(resolved);

			luaL_loadbuffer(L, file.data(), file.size(), file.c_str());

			return 1;
		}
		else
		{
			logger->error("Could not find file loaded from lua: '{}'", resolved);
			std::string formated = fmt::format(" Could not find file ('{}')", resolved);
			sol::stack::push(L, formated);
			return 1;
		}
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

	to.create_named_table("__loaded_modules");
	load((sol::table&)to.globals(), pkg);

	to.add_package_loader(LoadFileRequire, true);

}


void LuaCore::load(sol::table& to, const std::string& pkg)
{
	to["__pkg"] = pkg;
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
