#include "LuaSceneLib.h"
#include "game/GameState.h"
#include "game/scenes/LuaScene.h"

void LuaSceneLib::load_to(sol::table &table)
{
	table.set_function("load", [](const std::string& lua_path, sol::this_environment te)
	{
		// the scene is deleted by the scene manager!
		sol::environment& env = te;
		std::string this_pkg = env["__pkg"].get_or<std::string>("");
		auto* new_scene = new LuaScene(osp->game_state, lua_path, this_pkg);
		osp->game_state->load_scene(new_scene);
	});
}
