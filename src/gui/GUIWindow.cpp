#include "GUIWindow.h"

void GUIWindow::prepare(GUIInput* gui_input, GUISkin* skin)
{
	if(is_minimized())
	{
		return;
	}

	canvas.prepare(pos, size, gui_input);
}

void GUIWindow::draw(NVGcontext* vg, GUISkin* skin, glm::ivec4 def_scissor)
{
	skin->draw_window(vg, this);
	if(is_minimized())
	{
		return;
	}
	canvas.draw(vg, skin, def_scissor);
}

GUIWindow::GUIWindow() 
{
	focused = false;
	minimized = false;
	pinned = false;

	closeable = true;
	minimizable = true;
	pinable = true;
	moveable = true;
	resizeable = true;
	pin_passthrough = true;
	has_titlebar = true;
	min_size = glm::ivec2(50, 50);
	alpha = 1.0f;

}
