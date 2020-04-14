#pragma once
#include <nanovg/nanovg.h>
#include <ui/GUICanvas.h>

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

	float side_width = 0.22f;
	int minimum_side = 300;

	void do_gui(int width, int height);
	
	void prepare_def_panel(int width, int height);

	GUICanvas def_panel;

	EditorGUI();
};
