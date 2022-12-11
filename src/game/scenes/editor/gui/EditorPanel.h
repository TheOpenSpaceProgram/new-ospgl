#pragma once
#include <nanovg/nanovg.h>
#include <gui/GUISkin.h>
#include <gui/GUIInput.h>

class EditorScene;

// Base class for all editor panels
class EditorPanel
{
public:


	virtual void init(EditorScene* sc, NVGcontext* vg, GUISkin* skin) = 0;

	// TODO: These could be merged into a single function that checks gui_input != null or similar?
	// TODO: Maybe we could "standarize" GUI functions using a template or similar
	virtual void do_gui(int width, int panel_width, int height) = 0;
	virtual void prepare_gui(int width, int panel_width, int height, GUIInput* gui_input) = 0;

};