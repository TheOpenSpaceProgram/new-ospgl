#include "GUIWindow.h"
#include "GUIWindowManager.h"

void GUIWindow::position(GUIScreen *screen, GUISkin *skin)
{
	if(is_minimized())
	{
		return;
	}

	canvas->position_widgets(pos, size, screen);
}

void GUIWindow::prepare(GUIInput* gui_input, GUIScreen* screen)
{
	if(is_minimized())
	{
		return;
	}

	canvas->prepare(screen, gui_input);
}

void GUIWindow::draw(NVGcontext* vg, GUISkin* skin, glm::ivec4 def_scissor)
{
	skin->draw_window(vg, this);
	if(is_minimized())
	{
		return;
	}
	canvas->draw(vg, skin, def_scissor);
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
	min_size = glm::ivec2(50, 50);
	alpha = 1.0f;
	style = GUISkin::WindowStyle::NORMAL;

	canvas = std::make_shared<GUICanvas>();

}

void GUIWindow::close()
{
	wman->delete_window(this);
}
