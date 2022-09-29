#pragma once
#include "Scene.h"
#include <sol/sol.hpp>

class GameState;

// A scene that's implemented in lua. Most of the game scenes work like this
// Scenes are created INSIDE the universe lua state
// Scene lua files have two globals: universe and the function load_scene(string)
// load_scene is a soft-wrapper around load_scene which allows loading either
// lua scripts as scenes, or fixed name C++ scenes ("EDITOR").
class LuaScene : public Scene
{
public:

	std::string in_pkg;

	sol::environment env;
	sol::state* lua_state;

	void load() override;
	// Aditionally, lua gets passed osp->dt
	void update() override;
	void render() override;
	// Called the frame the scene is unloaded
	void unload() override;

	virtual ~LuaScene() {}

	LuaScene(GameState* in_state, const std::string& scene_script, const std::string& in_pkg);

};

