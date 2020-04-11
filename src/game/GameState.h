#pragma once
#include <universe/Universe.h>
#include <util/SerializeUtil.h>
#include "universe/Date.h"
#include "input/Input.h"
#include "scenes/Scene.h"

class OSP;

// Loads, runs, and stores a game state
class GameState
{
public:

	Scene* scene;
	Scene* to_delete;

	OSP* osp;
	Universe universe;

	void load(const cpptoml::table& from);
	void write(cpptoml::table& target) const;

	void update();

	void render();

	// Previous scene is deleted on next update
	void load_scene(Scene* new_scene);

	GameState(OSP* osp);
};


template<>
class GenericSerializer<GameState>
{
public:

	static void serialize(const GameState& what, cpptoml::table& target)
	{
		what.write(target);
	}

	static void deserialize(GameState& to, const cpptoml::table& from)
	{
		to.load(from);
	}
};
