#pragma once
#include "../Scene.h"
#include <universe/Universe.h>
#include <renderer/camera/SimpleCamera.h>
#include "FlightInput.h"
#include <renderer/lighting/SunLight.h>
#include <renderer/util/Skybox.h>

class FlightScene : public Scene
{
private:

	SimpleCamera camera;
	Universe* universe;
	GameState* game_state;

	Skybox sky;
	SunLight sun;

public:

	FlightInput input;

	virtual void load() override;
	virtual void update() override;
	virtual void render() override;
	// Called the frame the scene is unloaded
	virtual void unload() override;

	FlightScene() : sky(AssetHandle<Cubemap>("debug_system:skybox.png")) {}
};
