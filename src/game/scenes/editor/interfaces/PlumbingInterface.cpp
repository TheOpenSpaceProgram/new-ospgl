#include "PlumbingInterface.h"
#include "../EditorVehicleInterface.h"

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
	glm::vec4 span;
	span.x = (float)(viewport.x + viewport.z * view_size);
	span.y = (float)viewport.y;
	span.z = (float)(viewport.z * (1.0f - view_size));
	span.w = (float)viewport.w;

	// A portion of the screen is dedicated to viewing the vehicle, the other to plumbing edition
	// We draw a white background for the plumbing
	nvgBeginPath(vg);
	nvgRect(vg, span.x, span.y, span.z, span.w);
	nvgFillColor(vg, nvgRGB(255, 255, 255));
	nvgFill(vg);

	pb_editor.show_editor(vg, gui_input, span);

	return false;
}

glm::dvec4 PlumbingInterface::get_vehicle_viewport()
{
	return glm::dvec4(0.0, 0.0, (double)view_size, 1.0);
}

PlumbingInterface::PlumbingInterface(EditorVehicleInterface *edveh_int)
{
	pb_editor.veh = edveh_int->edveh->veh;
	view_size = 0.35f;


	Pipe pipe;
	pipe.waypoints.push_back(glm::vec2(5.5, 0.5));
	pipe.waypoints.push_back(glm::vec2(5.5, 5.0));
	pb_editor.veh->plumbing.pipes.push_back(pipe);
}

