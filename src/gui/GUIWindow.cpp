#include "GUIWindow.h"

void GUIWindow::prepare(GUIInput* gui_input, GUISkin* skin)
{
	// Adjust for titlebar and stuff
	canvas.prepare(pos, size, gui_input);
}

void GUIWindow::draw(NVGcontext* vg, GUISkin* skin, glm::ivec4 def_scissor)
{
	canvas.draw(vg, skin, def_scissor);
}
