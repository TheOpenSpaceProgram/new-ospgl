#include "GUIWindow.h"

void GUIWindow::prepare(GUIInput* gui_input, GUISkin* skin)
{

	canvas.prepare(pos, size, gui_input);
}

void GUIWindow::draw(NVGcontext* vg, GUISkin* skin, glm::ivec4 def_scissor)
{
	skin->draw_window(vg, this);
	canvas.draw(vg, skin, def_scissor);
}

GUIWindow::GUIWindow() 
{
	closeable = true;
	minimizable = true;
	pinable = false;
	moveable = true;
	resizeable = true;
	has_titlebar = true;
	min_size = glm::ivec2(50, 50);
	alpha = 1.0f;

}
