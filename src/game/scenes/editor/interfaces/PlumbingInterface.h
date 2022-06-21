#pragma once
#include "BaseInterface.h"
#include <game/plumbing/PlumbingEditor.h>

class EditorVehicleInterface;

class PlumbingInterface : public BaseInterface
{
private:

	Part* last_focused;
	size_t last_focused_i;

	float view_size;
	EditorVehicle* edveh;
	EditorVehicleInterface* edveh_int;

	void do_highlight();
	void do_3d_to_2d(GUIInput* gui_input, glm::dvec3 ray_start, glm::dvec3 ray_end);
	// Signal handler, called from PlumbingEditor's signal
	void do_2d_to_3d(Machine* m);

	void focus_pb_editor(Machine* m);

	void draw_icons(const Part* p, const PlumbingMachine* hovered, const std::vector<PlumbingMachine*> selected,
					const CameraUniforms& cu, glm::dvec4 viewport);

public:

	PlumbingEditor pb_editor;

	bool do_interface(const CameraUniforms& cu, glm::dvec3 ray_start, glm::dvec3 ray_end,
					  glm::dvec4 viewport, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin) override;
	void update(double dt) override;
	void leave() override;
	bool can_leave() override;
	glm::dvec4 get_vehicle_viewport() override;

	PlumbingInterface(EditorVehicleInterface* edveh_int);
};
