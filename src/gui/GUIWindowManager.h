#pragma once
#include "GUIWindow.h"

#include <list>

// This implements a simple stacking window manager
class GUIWindowManager
{
private:

	std::list<std::shared_ptr<GUIWindow>> windows;

public:

	std::shared_ptr<GUIWindow> focused;
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

	// As windows may be removed by the user at any time, we use weak_ptr
	// (In lua we end up returning a shared_ptr anyway!)
	std::weak_ptr<GUIWindow> create_window(glm::ivec2 pos = glm::ivec2(-1, -1),
											 glm::ivec2 size = glm::ivec2(-1, -1));
	void delete_window(GUIWindow* win);

	GUIWindowManager();

};
