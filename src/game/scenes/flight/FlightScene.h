#pragma once
#include "../Scene.h"
#include <universe/Universe.h>
#include <renderer/camera/SimpleCamera.h>
#include "FlightInput.h"
#include <renderer/lighting/SunLight.h>

class FlightScene : public Scene
{
private:

	SimpleCamera camera;
	Universe* universe;
	GameState* game_state;
	
	SunLight sun;

public:

	FlightInput input;

	virtual void load() override;
	virtual void update() override;
	virtual void render() override;
	// Called the frame the scene is unloaded
	virtual void unload() override;
};
