#include "LuaEntity.h"
#include "OSP.h"
#include <lua/LuaCore.h>
#include <game/GameState.h>

#include <utility>

void LuaEntity::enable_bullet(btDynamicsWorld *world)
{
	LuaUtil::call_function_if_present(env["enable_bullet"], world);
}

void LuaEntity::disable_bullet(btDynamicsWorld *world)
{
	LuaUtil::call_function_if_present(env["disable_bullet"], world);
}

glm::dvec3 LuaEntity::get_physics_origin()
{
	auto result = LuaUtil::call_function_if_present_returns<glm::dvec3>(env["get_physics_origin"]);
	return result.value_or(glm::dvec3(0, 0, 0));
}

glm::dvec3 LuaEntity::get_visual_origin()
{
	auto result = LuaUtil::call_function_if_present_returns<glm::dvec3>(env["get_visual_origin"]);
	return result.value_or(glm::dvec3(0, 0, 0));
}

double LuaEntity::get_physics_radius()
{
	auto result = LuaUtil::call_function_if_present_returns<double>(env["get_physics_radius"]);
	return result.value_or(0.0);
}

bool LuaEntity::is_physics_loader()
{
	auto result = LuaUtil::call_function_if_present_returns<bool>(env["is_physics_loader"]);
	return result.value_or(false);
}

void LuaEntity::update(double dt)
{
	LuaUtil::call_function_if_present(env["update"], dt);
}

void LuaEntity::physics_update(double pdt)
{
	LuaUtil::call_function_if_present(env["physics_update"], pdt);
}

void LuaEntity::init()
{
	LuaUtil::call_function_if_present(env["init"]);
}

bool LuaEntity::timewarp_safe()
{
	auto result = LuaUtil::call_function_if_present_returns<bool>(env["timewarp_safe"]);
	return result.value_or(true);
}

std::string LuaEntity::get_type()
{
	return type_str;
}

LuaEntity::LuaEntity(std::string script_path, std::string in_pkg, std::shared_ptr<cpptoml::table> init_toml)
{
	this->init_toml = std::move(init_toml);
	this->lua_state = &osp->game_state->universe.lua_state;

	auto[pkg, name] = osp->assets->get_package_and_name(script_path, in_pkg);
	type_str = pkg + ":" + name;

	env = sol::environment(*lua_state, sol::create, lua_state->globals());
	// We need to load LuaCore to it
	lua_core->load((sol::table&)env, pkg);
	env["universe"] = &osp->game_state->universe;
	env["entity"] = this;

	std::string full_path = osp->assets->res_path + pkg + "/" + name;
	auto result = (*lua_state).safe_script_file(full_path, env);
	if(!result.valid())
	{
		sol::error err = result;
		logger->fatal("Lua Error loading entity:\n{}", err.what());
	}

}
