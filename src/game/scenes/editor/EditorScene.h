#pragma once
#include "../Scene.h"
#include <nanovg/nanovg.h>
#include "EditorCamera.h"
#include "gui/EditorGUI.h"
#include "EditorVehicle.h"

#include <renderer/lighting/SunLight.h>

#include <assets/Model.h>

class EditorScene : public Scene
{
private:

	EditorCamera cam;
	EditorGUI gui;
	EditorVehicle vehicle;

	GPUModelNodePointer stack_model, radial_model, stack_radial_model, receive_model;	

	SunLight sun;

	void do_gui();

public:

	virtual void load() override;
	virtual void update() override;
	virtual void render() override;
	virtual void unload() override;

};
