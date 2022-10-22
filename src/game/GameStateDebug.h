#pragma once
#include <vector>
#include "renderer/camera/SimpleCamera.h"

class GameState;
class Entity;

// Allows accesing debug for all entities, scenes itself and a bunch of useful tools
// Works as a menubar
class GameStateDebug
{
private:
	GameState* g;

	std::vector<Entity*> shown_entity;
	Entity* centered_camera;

	bool terminal_shown;
	bool entities_shown;

	void do_terminal();
	void do_entities();
	void do_launcher();
	void do_assets();
	void do_scene();

	bool terminal_undocked;
	bool entities_undocked;
	bool assets_undocked;
	bool scene_undocked;

	static void do_docking_button(bool* val);

public:

	SimpleCamera cam;
	bool override_camera;

	bool show;
	bool allow_update;
	void update();
	void update_cam(double dt);

	GameStateDebug(GameState* gamestate);
};