#pragma once
#include "../EditorVehicle.h"
#include <gui/GUISkin.h>

class BaseInterface
{
public:

	virtual bool handle_input(const CameraUniforms& cu,
		glm::dvec3 ray_start, glm::dvec3 ray_end, GUIInput* gui_input) = 0;

	virtual void update(double dt) = 0;

	virtual void do_gui(NVGcontext* vg, GUISkin* gui_skin, glm::vec4 viewport) {}

	// Called when the interface changes by user input
	virtual void leave() = 0;

	virtual bool can_leave() = 0;
	

};