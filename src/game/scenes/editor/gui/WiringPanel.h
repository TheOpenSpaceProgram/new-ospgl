#pragma once
#include "EditorPanel.h"
#include <gui/GUICanvas.h>
#include <gui/layouts/GUISingleLayout.h>
#include <gui/widgets/GUIDropDown.h>
#include <assets/AssetManager.h>
#include <assets/Image.h>


class EditorVehicleInterface;
class EditorGUI;

class WiringPanel : EditorPanel
{
	EditorScene* scene;
	NVGcontext* vg;

	AssetHandle<Image> trash_image;


	GUICanvas panel;
	std::shared_ptr<GUIDropDown> group_dropdown;


	EditorGUI* edgui;

	void update_logical_groups();

public:
	void init(EditorScene* sc, NVGcontext* vg) override;
	void add_gui(int width, int panel_width, int height, GUIScreen* screen) override;

};
