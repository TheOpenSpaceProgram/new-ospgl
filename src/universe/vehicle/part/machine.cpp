#include "Machine.h"
#include <util/Logger.h>
#include <util/SerializeUtil.h>
#include <assets/AssetManager.h>

#include "../Vehicle.h"
#include "sol.hpp"


Machine::Machine(std::shared_ptr<cpptoml::table> init_toml, std::string cur_pkg)
{	
	this->init_toml = init_toml;
	logger->check(init_toml != nullptr, "Malformed init_toml");
	cpptoml::table& init_toml_p = *init_toml;

	std::string script_path;
	SAFE_TOML_GET_FROM(init_toml_p, script_path, "script", std::string);

	lua_state["machine"] = this;

	std::string old = assets->get_current_package();
	assets->set_current_package(cur_pkg);
	assets->load_script_to(lua_state, script_path);
	assets->set_current_package(old);

}

void Machine::pre_update(double dt)
{
	LuaUtil::call_function_if_present(lua_state, "pre_update", "machine pre_update", dt);
}

void Machine::update(double dt)
{
	LuaUtil::call_function_if_present(lua_state, "update", "machine update", dt);
}

void Machine::editor_update(double dt)
{
	LuaUtil::call_function_if_present(lua_state, "editor_update", "machine editor_update", dt);
}


void Machine::init(Part* in_part, Universe* in_universe)
{
	lua_state["part"] = in_part;
	lua_state["universe"] = in_universe;
	lua_state["vehicle"] = in_part->vehicle;

	this->in_part = in_part;

	LuaUtil::call_function_if_present(lua_state, "init", "machine init");
	
}


Machine::~Machine()
{
	logger->info("Ending machine");
	lua_state.collect_garbage();
}
