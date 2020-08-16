#pragma once
#include "BaseInterface.h"
#include <assets/BitmapFont.h>
#include <gui/GUIWindow.h>

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

	AssetHandle<BitmapFont> tiny_font;

	Machine* hovered;
	Machine* selected;
	std::vector<Machine*> selected_wired;

	void see_part(Part* p);

public:

	virtual void update(double dt) override;

	virtual void leave() override;

	virtual bool can_leave() override;
	
	virtual bool do_interface(const CameraUniforms& cu, glm::dvec3 ray_start, glm::dvec3 ray_end,
			glm::dvec4 viewport, NVGcontext* vg, GUIInput* gui_input, GUISkin* gui_skin) override;
	
	WireInterface(EditorVehicleInterface* edveh_int);

};
