#pragma once
#include "EditorPanel.h"
#include "universe/vehicle/plumbing/PlumbingMachine.h"

#include <gui/widgets/GUIImageButton.h>
#include <gui/layouts/GUIListLayout.h>
#include <gui/layouts/GUIVerticalLayout.h>
#include <gui/skins/SimpleSkin.h>
#include <gui/GUICanvas.h>
#include <universe/vehicle/part/Part.h>

class EditorVehicleInterface;

// We provide a list of plumbing only parts (may be user provided),
// and some useful tools for plumbing layout.
class PlumbingPanel : EditorPanel
{
private:
	GUISkin* gui_skin;
	NVGcontext* vg;
	EditorScene* scene;
	EditorVehicleInterface* edveh_int;
	GUIInput* gui_input;

public:

	Part* target_part;
	GUICanvas def_panel;
	GUIListLayout* part_list;
	int part_margin = 4;

	void init(EditorScene* sc, NVGcontext* vg, GUISkin* skin) override;

	void prepare_gui(int width, int panel_width, int height, GUIInput* gui_input) override;
	void do_gui(int width, int panel_width, int height) override;
	void create_machine(std::string database_id);

};


