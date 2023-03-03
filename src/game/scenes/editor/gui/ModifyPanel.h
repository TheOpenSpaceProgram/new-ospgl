#pragma once
#include "EditorPanel.h"


class EditorGUI;
class EditorScene;
// Shows a list of pieces / parts on the vehicle, alongside the utils toolbox and symmetry toolbox
// By default, clicking parts shows context menus, but all tools have certain interactions
class ModifyPanel : public EditorPanel
{
private:
	EditorScene* sc;
	EditorGUI* edgui;
	GUIInput* gui_input;
	NVGcontext* vg;

public:
	void init(EditorScene* sc, NVGcontext* vg) override;
	void add_gui(int width, int panel_width, int height, GUIScreen* screen) override;

	ModifyPanel();

};
