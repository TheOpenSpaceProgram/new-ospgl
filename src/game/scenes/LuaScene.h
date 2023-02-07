#pragma once
#include "Scene.h"
#include <sol/sol.hpp>
#include <gui/skins/SimpleSkin.h>
#include "renderer/camera/LuaCamera.h"

class GameState;

// A scene that's implemented in lua. Most of the game scenes work like this
// Scenes are created INSIDE the universe lua state
// Scene lua files have the following globals: osp, gui_input
// Furthermore, they may implement load, update, render, unload and get_camera_uniforms functions
// For the get_camera_uniforms, you must return a CameraUniforms object (see examples in core/)
// Passed arguments can be received on "load"
class LuaScene : public Scene
{
public:

	std::vector<sol::object> to_pass_args;

	std::string in_pkg;
	std::string name;
	LuaCamera cam;

	sol::environment env;
	sol::state* lua_state;

	void load() override;
	// It's your responsability to call universe->update(dt) (or not)
	// Gets passed osp->dt
	void pre_update() override;
	// Aditionally, lua gets passed osp->dt
	void update() override;
	void physics_update(double bdt) override;
	void render() override;
	// Called the frame the scene is unloaded
	void unload() override;

	void do_imgui_debug() override;

	std::string get_name() override;

	virtual ~LuaScene() {}

	// This method of passing args is "orthodox", we follow sol documentation
	// (It could seem like it's a hack)
	LuaScene(GameState* in_state, const std::string& scene_script, const std::string& in_pkg,
		  std::vector<sol::object> args);

};

