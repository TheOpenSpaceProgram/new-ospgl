#pragma once
#include <nanovg/nanovg.h>
#include <gui/GUICanvas.h>
#include <gui/GUIInput.h>

class EditorScene;

// The side-pane can display a lot of stuff, but usually it displays
// the C++ implemented part list and editor controls. Parts can draw a
// custom interface there, use their context menu, or pop up a full
// blown, lua controlled, gui
class EditorGUI
{
private:

	int prev_width, prev_height;

public:
	NVGcontext* vg;

	float side_width = 0.15f;
	int minimum_side = 256;

	void do_gui(int width, int height, GUIInput* gui_input);
	
	GUICanvas def_panel;

	void init(EditorScene* scene);

};
