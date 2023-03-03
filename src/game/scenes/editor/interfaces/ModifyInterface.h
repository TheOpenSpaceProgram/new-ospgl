#pragma once
#include "BaseInterface.h"

class EditorVehicleInterface;

class ModifyInterface : public BaseInterface
{
private:
	EditorVehicleInterface* edveh_int;
	EditorVehicle* edveh;

public:


	void update(double dt) override;

	bool do_interface(const CameraUniforms& cu, glm::dvec3 ray_start, glm::dvec3 ray_end,
							  glm::dvec4 viewport, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin) override;

	// Called when the interface changes by user input
	void leave() override;

	bool can_leave() override;

	ModifyInterface(EditorVehicleInterface* edveh_int);
};
