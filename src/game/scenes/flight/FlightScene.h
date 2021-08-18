#pragma once
#include "../Scene.h"
#include <universe/Universe.h>
#include <renderer/camera/SimpleCamera.h>
#include "FlightInput.h"
#include <renderer/lighting/SunLight.h>
#include <renderer/util/Skybox.h>

#include "gui/FlightGUI.h"

class FlightScene : public Scene
{
private:

	friend class FlightGUI;

	SimpleCamera camera;
	Universe* universe;
	GameState* game_state;

	Skybox sky;
	SunLight sun;

	void do_gui();
	void prepare_gui();

public:

	FlightInput input;
	FlightGUI gui;

	void load() override;
	void update() override;
	void render() override;
	// Called the frame the scene is unloaded
	void unload() override;

	FlightScene() : sky(AssetHandle<Cubemap>("debug_system:skybox.png")) {}
};
