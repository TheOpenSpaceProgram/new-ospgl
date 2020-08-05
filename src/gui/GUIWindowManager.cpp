#include "GUIWindowManager.h"


void GUIWindowManager::prepare(GUIInput* gui_input, GUISkin* skin)
{
	if(windows.size() == 0)
	{
		return;
	}

	bool mouse_down = gui_input->mouse_down(0) || gui_input->mouse_down(1) || gui_input->mouse_down(2);
	bool found_focus = false;
	GUIWindow* new_top = nullptr;

	for(GUIWindow* w : windows)
	{
		if(mouse_down && gui_input->mouse_inside(w->pos, w->size) && !found_focus)
		{
			if(w != focused)
			{
				w->focused = true;
				focused->focused = false;
				found_focus = true;
				new_top = w;
				focused = w;
			}
		}
	}

	// Moves new_top to the front
	windows.remove(new_top);
	windows.push_front(new_top);

	bool original_block_mouse = gui_input->ext_mouse_blocked;
	bool original_block_keyboard = gui_input->ext_keyboard_blocked;

	for(GUIWindow* w : windows)
	{
		if(w != focused)
		{
			// Block user input
			gui_input->ext_mouse_blocked = true;
			gui_input->ext_keyboard_blocked = true;
		}
		w->prepare(gui_input, skin);
	}

	gui_input->ext_mouse_blocked = original_block_mouse;
	gui_input->ext_keyboard_blocked = original_block_keyboard;
}

void GUIWindowManager::draw(NVGcontext* vg, GUISkin* skin)
{

}

GUIWindow* GUIWindowManager::create_window()
{
	GUIWindow* n_window = new GUIWindow();

	n_window->focused = false;

	windows.push_back(n_window);
	if(windows.size() == 1)
	{
		focused = n_window;
		n_window->focused = true;
	}

	return n_window;
}

void GUIWindowManager::delete_window(GUIWindow* win)
{
	windows.remove(win);
	if(focused == win)
	{
		focused = nullptr;
	}

	delete win;
}
