#pragma once
#include <string>
#include "GUICanvas.h"
#include "Signal.h"
#include "glm/fwd.hpp"

class GUIWindowManager;

// A window offers a wide variety of ways to hold a canvas. They must be 
// created on a window manager to handle stacking
// Most non-fixed GUIs are drawn through this
class GUIWindow
{

friend class GUIWindowManager;

private:
	bool focused;
public:


	
	// These are in screen-space pixels. They may be adjusted by the window
	// manager if neccesary (only position). The size of the window will be bigger
	// due to decorations
	glm::ivec2 pos;
	glm::ivec2 size;

	bool close_hovered;
	bool minimize_hovered;
	bool pin_hovered;

	bool has_titlebar;
	std::string title;

	// These are only valid on windows with a titlebar
	bool closeable;
	// A minimized window shows just the titlebar
	bool minimizable;
	// A pinned window is always on top
	bool pinable;
	bool resizeable;
	bool moveable;

	// The canvas is automatically handled by the window
	GUICanvas canvas;

	// Useful for cleaning up (widgets are auto-deleted with the canvas!)
	Signal<void(GUIWindow*)> on_close;

	// We need the GUISkin on prepare to adjust sizings
	void prepare(GUIInput* gui_input, GUISkin* skin);
	void draw(NVGcontext* vg, GUISkin* skin, glm::ivec4 def_scissor);

	bool is_focused() { return focused; }

	GUIWindow();
};
