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
	EditorScene* scene;
	NVGcontext* vg;

	AssetHandle<Image> trash_image;

	GUICanvas panel;
	GUISingleLayout* trash_area_layout;
	GUIImageButton trash_button;

	EditorGUI* edgui;

public:
	void init(EditorScene* sc, NVGcontext* vg) override;
	void add_gui(int width, int panel_width, int height, GUIScreen* screen) override;

};
