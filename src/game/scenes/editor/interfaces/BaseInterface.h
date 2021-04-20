#pragma once
#include "../EditorVehicle.h"
#include <gui/GUISkin.h>

class BaseInterface
{
public:

	virtual void update(double dt) = 0;

	virtual bool do_interface(const CameraUniforms& cu, glm::dvec3 ray_start, glm::dvec3 ray_end,
			glm::dvec4 viewport, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin) = 0;

	// Get the viewport of the vehicle, fractional coordinates relative to max size (viewport)
	// Useful to implement "opaque" interfaces so the vehicle remains centered and we save on GPU usage
	virtual glm::dvec4 get_vehicle_viewport() { return glm::vec4(0.0, 0.0f, 1.0f, 1.0f); }

	// Called when the interface changes by user input
	virtual void leave() = 0;

	virtual bool can_leave() = 0;
	

};
