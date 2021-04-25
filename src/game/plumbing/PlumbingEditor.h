#pragma once
#include <gui/GUIInput.h>
#include <nanovg/nanovg.h>
#include <universe/vehicle/Vehicle.h>

// Display and editing of the plumbing of a vehicle
// Maybe used in the editor to set up plumbing, and viewed
// during flight (although cockpit instruments should be preferred)
// To use as a view-only widget, wrap it around and give gui_input as nullptr
// it will then ignore all inputs and just display a graphic
class PlumbingEditor
{
private:

	glm::dvec2 last_click;
	glm::vec2 last_center;
	bool in_drag;

	// Draws a 1 unit grid. If zoom is too
	void draw_grid(NVGcontext* vg, glm::vec4 span);

public:

	Vehicle* veh;
	glm::vec2 cam_center;
	// How many pixels is each unit?
	int zoom;

	// This modifies (and reads) gui_input if it blocks mouse or similar so be aware of it!
	// if gui_input is nullptr, it will be view only
	// We don't draw the background
	// span is a rectangle, (x,y, width, height)
	void show_editor(NVGcontext* vg, GUIInput* gui_input, glm::vec4 span);

	PlumbingEditor();

};
