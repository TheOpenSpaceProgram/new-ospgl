#pragma once
#include "GUIWindow.h"

#include <list>

// This implements a simple stacking window manager
class GUIWindowManager
{
private:

	std::list<GUIWindow*> windows;

public:

	GUIWindow* focused;

	// In pixels, allows bounding the windows to a certain viewport
	glm::ivec4 viewport;


	void prepare(GUIInput* gui_input, GUISkin* skin);
	void draw(NVGcontext* vg, GUISkin* skin);

	GUIWindow* create_window();
	void delete_window(GUIWindow* win);

};
