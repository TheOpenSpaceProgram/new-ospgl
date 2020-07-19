#pragma once
#include "../EditorVehicle.h"

class BaseInterface
{
public:

	virtual bool handle_input(const CameraUniforms& cu,
		glm::dvec3 ray_start, glm::dvec3 ray_end, GUIInput* gui_input) = 0;

	virtual void update(double dt) = 0;

	// Called when the interface changes by user input
	virtual void leave() = 0;

	virtual bool can_leave() = 0;
	

};