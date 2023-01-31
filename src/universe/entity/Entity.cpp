#include "Entity.h"
#include "../Universe.h"
#include <lua/LuaCore.h>
#include <game/GameState.h>
#include <utility>
#include <game/scenes/flight/InputContext.h>

void Entity::enable_bullet(btDynamicsWorld *world)
{
	LuaUtil::call_function_if_present(env["enable_bullet"], world);
}

void Entity::disable_bullet(btDynamicsWorld *world)
{
	LuaUtil::call_function_if_present(env["disable_bullet"], world);
}

glm::dvec3 Entity::get_physics_origin()
{
	auto result = LuaUtil::call_function_if_present_returns<glm::dvec3>(env["get_physics_origin"]);
	return result.value_or(glm::dvec3(0, 0, 0));
}

glm::dvec3 Entity::get_visual_origin()
{
	auto result = LuaUtil::call_function_if_present_returns<glm::dvec3>(env["get_visual_origin"]);
	return result.value_or(glm::dvec3(0, 0, 0));
}

double Entity::get_physics_radius()
{
	auto result = LuaUtil::call_function_if_present_returns<double>(env["get_physics_radius"]);
	return result.value_or(0.0);
}

bool Entity::is_physics_loader()
{
	auto result = LuaUtil::call_function_if_present_returns<bool>(env["is_physics_loader"]);
	return result.value_or(false);
}

void Entity::update(double dt)
{
	LuaUtil::call_function_if_present(env["update"], dt);
}

void Entity::physics_update(double pdt)
{
	LuaUtil::call_function_if_present(env["physics_update"], pdt);
}

void Entity::init()
{
	LuaUtil::call_function_if_present(env["init"]);
}

bool Entity::timewarp_safe()
{
	auto result = LuaUtil::call_function_if_present_returns<bool>(env["timewarp_safe"]);
	return result.value_or(true);
}

std::string Entity::get_type()
{
	return type_str;
}

Entity::Entity(std::string script_path, std::string in_pkg, std::shared_ptr<cpptoml::table> init_toml,
					 std::vector<sol::object> args, bool is_create)
{
	this->init_toml = std::move(init_toml);
	this->lua_state = &osp->game_state->universe.lua_state;

	auto[pkg, name] = osp->assets->get_package_and_name(script_path, in_pkg);
	type_str = pkg + ":" + name;

	env = sol::environment(*lua_state, sol::create, lua_state->globals());
	// We need to load LuaCore to it
	lua_core->load((sol::table&)env, pkg);
	env["entity"] = this;
	env["osp"] = osp;

	std::string full_path = osp->assets->res_path + pkg + "/" + name;
	auto result = (*lua_state).safe_script_file(full_path, env);
	if(!result.valid())
	{
		sol::error err = result;
		logger->fatal("Lua Error loading entity:\n{}", err.what());
	}

	if(is_create)
	{
		LuaUtil::call_function_if_present_args(env["create"], args);
	}

}

void Entity::save(cpptoml::table &to)
{
	LuaUtil::call_function_if_present(env["save"], to);
}

void Entity::setup(Universe* universe, int64_t uid)
{
	this->universe = universe;
	this->uid = uid;
	init();
}

Entity::~Entity()
{

}

Entity* Entity::load(std::string type, std::shared_ptr<cpptoml::table> toml)
{
	Entity* n_ent = new Entity(type, "core", toml, {}, false);
	return n_ent;
}

void Entity::deferred_pass(CameraUniforms &cu, bool is_env_map)
{
	LuaUtil::call_function_if_present(env["deferred_pass"], cu, is_env_map);
}

void Entity::forward_pass(CameraUniforms &cu, bool is_env_map)
{
	LuaUtil::call_function_if_present(env["forward_pass"], cu, is_env_map);
}

void Entity::gui_pass(CameraUniforms &cu)
{
	LuaUtil::call_function_if_present(env["gui_pass"], cu);
}

void Entity::shadow_pass(ShadowCamera &cu)
{
	LuaUtil::call_function_if_present(env["shadow_pass"], cu);
}

void Entity::far_shadow_pass(ShadowCamera &cu)
{
	LuaUtil::call_function_if_present(env["far_shadow_pass"], cu);
}

bool Entity::needs_deferred_pass()
{
	return LuaUtil::call_function_if_present_returns<bool>(env["needs_deferred_pass"]).value_or(false);
}

bool Entity::needs_forward_pass()
{
	return LuaUtil::call_function_if_present_returns<bool>(env["needs_forward_pass"]).value_or(false);
}

bool Entity::needs_gui_pass()
{
	return LuaUtil::call_function_if_present_returns<bool>(env["needs_gui_pass"]).value_or(false);
}

bool Entity::needs_shadow_pass()
{
	return LuaUtil::call_function_if_present_returns<bool>(env["needs_shadow_pass"]).value_or(false);
}

bool Entity::needs_far_shadow_pass()
{
	return LuaUtil::call_function_if_present_returns<bool>(env["needs_far_shadow_pass"]).value_or(false);
}

bool Entity::needs_env_map_pass()
{
	return LuaUtil::call_function_if_present_returns<bool>(env["needs_env_map_pass"]).value_or(false);
}

void Entity::do_debug_imgui()
{
	LuaUtil::call_function_if_present(env["do_debug_imgui"]);
}

InputContext* Entity::get_input_ctx()
{
	return LuaUtil::call_function_if_present_returns<InputContext*>(env["get_input_ctx"]).value_or(nullptr);
}

glm::dvec3 Entity::get_velocity()
{
	return LuaUtil::call_function_if_present_returns<glm::dvec3>(env["get_velocity"]).value_or(glm::dvec3(0, 0, 0));
}
