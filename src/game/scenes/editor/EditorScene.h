#pragma once
#include "../Scene.h"
#include <nanovg/nanovg.h>
#include "EditorCamera.h"
#include "gui/EditorGUI.h"
#include "EditorVehicle.h"

#include <renderer/lighting/SunLight.h>

class EditorScene : public Scene
{
private:

	EditorCamera cam;
	EditorGUI gui;
	EditorVehicle vehicle;

	SunLight sun;


	void do_gui();

public:

	virtual void load() override;
	virtual void update() override;
	virtual void render() override;
	virtual void unload() override;

};
