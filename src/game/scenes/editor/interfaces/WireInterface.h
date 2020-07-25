#pragma once
#include "BaseInterface.h"

class EditorVehicleInterface;

class WireInterface : public BaseInterface
{
private:
	EditorVehicle* edveh;
	EditorScene* scene;
	EditorVehicleInterface* edveh_int;

	std::vector<std::pair<Part*, std::vector<Machine*>>> visible_machines;
	// We story a copy for the GUI drawing
	CameraUniforms cu;

	Machine* hovered;
	Machine* selected;
	std::vector<Machine*> selected_wired;

	void see_part(Part* p);

public:

	virtual bool handle_input(const CameraUniforms& cu, glm::dvec3 ray_start, 
		glm::dvec3 ray_end, GUIInput* gui_input) override;

	virtual void update(double dt) override;

	virtual void leave() override;

	virtual bool can_leave() override;
	
	virtual void do_gui(NVGcontext* vg, GUISkin* gui_skin, GUIInput* gui_input, glm::vec4 viewport) override;
	
	WireInterface(EditorVehicleInterface* edveh_int);

};