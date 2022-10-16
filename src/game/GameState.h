#pragma once
#include <universe/Universe.h>
#include <util/SerializeUtil.h>
#include "universe/Date.h"
#include "scenes/Scene.h"

class OSP;

// Loads, runs, and stores a game state
// It consists of a save.toml, and many extra files stored in its folder in saves/
// for example, in flight vehicles are stored in in_flight/, saved vehicles in vehicles/,
// screenshots in screenshots/ etc...
class GameState
{
private:
	// only valid during init() and load()
	std::unordered_map<Entity*, int64_t> ent_to_id;

public:

	// Path to save / load stuff from this save state (udata/path/this)
	std::string path;
	// Which packages should be loaded to play this save?
	std::vector<std::string> used_packages;

	Scene* scene;
	Scene* to_delete;

	Universe universe;

	void write(cpptoml::table& target) const;

	void update();

	void render();
	void init();

	// Previous scene is deleted on next update
	void load_scene(Scene* new_scene);

	// Creates a new save given a planetary system (and the "core" package)
	static GameState* create_empty(const std::string& planetary_system_package);
	static GameState* create_main_menu(const std::string& skip_to_save);
	static GameState* load(const std::string& path);

	void write();

	GameState();
};


