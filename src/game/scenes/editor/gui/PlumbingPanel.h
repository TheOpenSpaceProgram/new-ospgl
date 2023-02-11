#pragma once
#include "EditorPanel.h"
#include "universe/vehicle/plumbing/PlumbingMachine.h"

#include <gui/widgets/GUIImageButton.h>
#include <gui/widgets/GUITextButton.h>
#include <gui/layouts/GUIListLayout.h>
#include <gui/layouts/GUISingleLayout.h>
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
	NVGcontext* vg;
	EditorScene* scene;
	EditorVehicleInterface* edveh_int;
	std::shared_ptr<GUITextButton> trashcan_button;

public:

	Part* target_part;
	GUICanvas def_panel;
	std::shared_ptr<GUIListLayout> part_list;
	std::shared_ptr<GUISingleLayout> trashcan;

	int part_margin = 4;

	void init(EditorScene* sc, NVGcontext* vg) override;

	void add_gui(int width, int panel_width, int height, GUIScreen* screen) override;
	void create_machine(std::string database_id);

};


