#pragma once
#include <gui/GUIInput.h>
#include <nanovg/nanovg.h>
#include <universe/vehicle/Vehicle.h>

// Display and editing of the plumbing of a vehicle
// Maybe used in the editor to set up plumbing, and viewed
// during flight (although cockpit instruments should be preferred)
// To use as a view-only widget, wrap it around and give gui_input as nullptr
// it will then ignore all inputs and just display a graphic
// TODO: Logic and drawing is already separated, make it a GUIWindget?
class PlumbingEditor
{
private:

	float pipe_end_radius = 0.18f;

	Machine* hovered;

	bool in_selection;
	bool in_pipe_drag;
	// Pipe drags start by clicking a pipe end-cap. A pipe is automatically
	// created when clicking on ports and dragging atleast one square. Pipes can
	// be deleted by shortening them to 0 length
	bool in_machine_drag;
	glm::vec2 mouse_start;
	glm::vec2 mouse_current;
	// We allow shift-selection and also box selection
	std::vector<Machine*> selected;
	std::vector<Machine*> drag_conflicts;

	glm::dvec2 last_click;
	glm::vec2 last_center;
	bool in_drag;

	static bool is_inside_and_not_blocked(GUIInput* gui_input, glm::vec4 span);
	bool update_mouse(GUIInput* gui_input, glm::vec4 span);
	bool update_selection(GUIInput* gui_input, glm::vec4 span);
	bool update_dragging(GUIInput* gui_input, glm::vec2 mpos);
	bool update_pipes(GUIInput* gui_input, glm::vec4 span);
	// Returns mouse pos in corrected coordinates
	glm::vec2 get_mouse_pos(glm::vec4 span) const;

	// Draws a 1 unit grid. If zoom is too
	void draw_grid(NVGcontext* vg, glm::vec4 span) const;
	void draw_machines(NVGcontext* vg, glm::vec4 span) const;
	void draw_selection(NVGcontext* vg, glm::vec4 span) const;
	void draw_pipes(NVGcontext* vg, glm::vec4 span) const;
	void draw_collisions(NVGcontext* vg, glm::vec4 span) const;
	void draw_pipe_cap(NVGcontext* vg, glm::vec2 pos) const;

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
