#pragma once

#include <assets/AssetManager.h>
#include <gui/skins/SimpleSkin.h>
#include <gui/widgets/GUIImageButton.h>
#include <gui/GUIWindowManager.h>
#include <game/plumbing/PlumbingEditorWidget.h>

class FlightScene;

class FlightGUI
{
public:

	SimpleSkin skin;
	NVGcontext* vg;

	GUIWindowManager window_manager;
	PlumbingEditorWidget* pb_editor;
	GUIWindow* plumbing;

	void init(FlightScene* scene);
	void prepare_gui(int width, int height, GUIInput* gui_input);
	void do_gui(int width, int height);

};

