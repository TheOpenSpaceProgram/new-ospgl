#pragma once
#include <gui/GUIInput.h>
#include <gui/GUISkin.h>
#include <nanovg/nanovg.h>
#include <universe/vehicle/Vehicle.h>
#include <util/Signal.h>

// Display and editing of the plumbing of a vehicle
// Maybe used in the editor to set up plumbing, and viewed
// during flight (although cockpit instruments should be preferred)
// To use as a view-only widget, wrap it around and give gui_input as nullptr
// it will then ignore all inputs and just display a graphic
// TODO: Logic and drawing is already separated, make it a GUIWindget?
class PlumbingEditor
{
private:

	// For ease of access
	GUISkin* skin;

	// For rotation you must click and un-click fast enough
	double time_held;
	double max_time_for_rotation = 0.2;
	float port_radius = 0.18f;

	MachinePlumbing* hovered;

	FluidPort* hovered_port;
	// We are hovering an unfinished pipe endpoint if this is not negative, or... see in_pipe_drag
	int hovering_pipe;
	// If true, hovering_pipe contains the pipe we are working on, and the pipe-dragging UI will be at play
	bool in_pipe_drag;


	bool in_selection;
	// Pipe drags start by clicking a pipe end-cap. A pipe is automatically
	// created when clicking on ports and dragging atleast one square. Pipes can
	// be deleted by shortening them to 0 length
	bool in_machine_drag;
	glm::vec2 mouse_start;
	glm::vec2 mouse_current;
	// We allow shift-selection and also box selection
	std::vector<MachinePlumbing*> selected;
	std::vector<MachinePlumbing*> drag_conflicts;

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
	void draw_port(NVGcontext* vg, glm::vec2 pos, bool hovered = false) const;
	void draw_pipe_cap(NVGcontext* vg, glm::vec2 pos) const;
	void draw_tooltip(NVGcontext* vg, glm::vec4 span) const;

	// Junctions of more than 4 ports are drawn as a series of connected junctions
	void draw_junction(NVGcontext* vg, glm::vec2 pos, size_t port_count, bool flip_three=false) const;

	// Handles both AABBs and ports
	void handle_hovering(GUIInput* gui_input, glm::vec2 mpos);

	void pipe_line_to(NVGcontext* vg, glm::vec2 pos, glm::vec2 old_pos, float flow) const;

public:

	bool allow_editing;
	bool allow_dragging;
	// Shows direction of flow in every pipe
	bool show_flow_direction;
	// Shows detailed tooltip for ports on hovering them
	bool allow_tooltip;

	Signal<void(Machine*&)> on_middle_click;

	Vehicle* veh;
	glm::vec2 cam_center;
	// How many pixels is each unit?
	int zoom;

	// This modifies (and reads) gui_input if it blocks mouse or similar so be aware of it!
	// if gui_input is nullptr, it will be view only
	// We don't draw the background
	// span is a rectangle, (x,y, width, height)
	void prepare(GUIInput* gui_input, glm::vec4 span);
	void do_editor(NVGcontext* vg, glm::vec4 span, GUISkin* skin);

	const MachinePlumbing* get_hovered() const { return hovered; }
	const std::vector<MachinePlumbing*> get_selected() const { return selected; }

	PlumbingEditor();

};
