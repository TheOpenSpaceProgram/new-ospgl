#include "PlumbingInterface.h"

bool PlumbingInterface::can_leave()
{
	return true;
}

void PlumbingInterface::leave()
{
}

void PlumbingInterface::update(double dt)
{

}

bool PlumbingInterface::do_interface(const CameraUniforms& cu, glm::dvec3 ray_start, glm::dvec3 ray_end,
								   glm::dvec4 viewport, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin)
{
	// A portion of the screen is dedicated to viewing the vehicle, the other to plumbing edition
	nvgBeginPath(vg);
	nvgRect(vg, viewport.x + viewport.z * view_size, viewport.y, viewport.z* (1.0f - view_size), viewport.w);
	nvgFillColor(vg, nvgRGB(255, 255, 255));
	nvgFill(vg);

	return false;
}

glm::dvec4 PlumbingInterface::get_vehicle_viewport()
{
	return glm::dvec4(0.0, 0.0, (double)view_size, 1.0);
}

PlumbingInterface::PlumbingInterface()
{
	view_size = 0.5f;
}
