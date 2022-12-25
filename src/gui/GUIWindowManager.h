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
	bool dragging;
	bool resizing;
	GUISkin::ResizePoint resize_point;
	glm::vec2 drag_point;
	glm::ivec2 pos_0;
	glm::ivec2 size_0;

	// In pixels, allows bounding the windows to a certain viewport
	glm::ivec4 viewport;

	bool collapse_horizontal;
	// Location for collapsed windows, titles are shown stacked vertically / horizontally
	// (x, y, w, h)
	// If items go out of bounds, an scrollbar is shown (this is handled as a GUICanvas)
	glm::ivec4 collapse_location;

	glm::ivec2 default_pos;
	glm::ivec2 default_size;


	void position(GUIScreen* screen);
	void prepare(GUIInput* gui_input, GUIScreen* screen);
	void draw(NVGcontext* vg, GUIScreen* screen);

	// Use negative values to place on default pos / size
	// You may pass a pointer (use nullptr if you dont want it)
	// for it to be set to nullptr on window deletion.
	GUIWindow* create_window(GUIWindow** erase_ptr = nullptr,
			glm::ivec2 pos = glm::ivec2(-1, -1), glm::ivec2 size = glm::ivec2(-1, -1));
	void delete_window(GUIWindow* win);

	GUIWindowManager();

};
