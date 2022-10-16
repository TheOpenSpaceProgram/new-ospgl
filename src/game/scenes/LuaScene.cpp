#include "LuaScene.h"
#include "lua/LuaCore.h"
#include "../GameState.h"
#include "renderer/Renderer.h"

LuaScene::LuaScene(GameState* in_state, const std::string& scene_script, const std::string& in_pkg,
				   std::vector<sol::object> args) :
	cam(&this->env)
{
	this->to_pass_args = args;
	this->in_pkg = in_pkg;
	this->lua_state = &in_state->universe.lua_state;
	osp->renderer->cam = &cam;

	auto[pkg, name] = osp->assets->get_package_and_name(scene_script, in_pkg);

	logger->info("Loading lua scene: {}:{}", pkg, name);

	env = sol::environment(*lua_state, sol::create, lua_state->globals());
	// We need to load LuaCore to it
	lua_core->load((sol::table&)env, pkg);
	env["osp"] = osp;

	std::string full_path = osp->assets->res_path + pkg + "/" + name;
	auto result = (*lua_state).safe_script_file(full_path, env);
	if(!result.valid())
	{
		sol::error err = result;
		logger->fatal("Lua Error loading scene:\n{}", err.what());
	}


}

void LuaScene::load()
{
	LuaUtil::call_function_if_present_args(env["load"], to_pass_args);
	to_pass_args.clear();
}

void LuaScene::update()
{
	LuaUtil::call_function_if_present(env["update"], osp->dt);
}

void LuaScene::render()
{
	LuaUtil::call_function_if_present(env["render"]);
}

void LuaScene::unload()
{
	LuaUtil::call_function_if_present(env["unload"]);
}
