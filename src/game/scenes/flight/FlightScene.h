#pragma once
#include "../Scene.h"
#include <universe/Universe.h>
#include <renderer/camera/SimpleCamera.h>

class FlightScene : public Scene
{
private:

	SimpleCamera camera;

public:

	virtual void load() override;
	virtual void update() override;
	virtual void render() override;
	// Called the frame the scene is unloaded
	virtual void unload() override;
};
