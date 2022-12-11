#pragma once
#include "EditorPanel.h"
#include <gui/GUICanvas.h>
#include <gui/layouts/GUISingleLayout.h>
#include <gui/widgets/GUIImageButton.h>
#include <assets/AssetManager.h>
#include <assets/Image.h>

class EditorVehicleInterface;
class EditorGUI;

class WiringPanel : EditorPanel
{
	GUISkin* gui_skin;
	NVGcontext* vg;

	AssetHandle<Image> trash_image;

	GUICanvas panel;
	GUISingleLayout* trash_area_layout;
	GUIImageButton trash_button;

	EditorGUI* edgui;
	GUIInput* gui_input;

public:
	void init(EditorScene* sc, NVGcontext* vg, GUISkin* skin) override;
	void prepare_gui(int width, int panel_width, int height, GUIInput* gui_input) override;
	void do_gui(int width, int panel_width, int height) override;

};
