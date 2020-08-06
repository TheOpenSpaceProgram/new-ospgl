#include "GUIWindowManager.h"
#include <util/InputUtil.h>

void GUIWindowManager::prepare(GUIInput* gui_input, GUISkin* skin)
{
	if(windows.size() == 0)
	{
		return;
	}

	bool mouse_down = gui_input->mouse_down(0) || gui_input->mouse_down(1) || gui_input->mouse_down(2);
	bool found_focus = false;
	GUIWindow* new_top = nullptr;

	bool any_hovered = false;
	// We iterate in reverse (painter's algorithm is used for rendering)
	for(auto it = windows.rbegin(); it != windows.rend(); it++)
	{
		glm::ivec2 mouse_pos = input->mouse_pos;
		GUIWindow* w = *it;
		w->close_hovered = false;
		w->minimize_hovered = false;
		w->pin_hovered = false;
		bool mouse_inside = false;
		glm::ivec4 aabb = skin->get_window_aabb(w);
		if(gui_input->mouse_inside(glm::vec2(aabb.x, aabb.y), glm::vec2(aabb.z, aabb.w)))
		{
			any_hovered = true;
			mouse_inside = true;

			if(w->closeable && skin->can_close_window(w, mouse_pos))
			{
				w->close_hovered = true;	
			}
			else if(w->minimizable && skin->can_minimize_window(w, mouse_pos))
			{
				w->minimize_hovered = true;
			}
			else if(w->pinable && skin->can_pin_window(w, mouse_pos))
			{
				w->pin_hovered = true;
			}
		}

		if(mouse_down && mouse_inside && !found_focus)
		{
			bool focus = true;
			// This can be one of many actions

			found_focus = true;
			if(w != focused && focus)
			{
				w->focused = true;
				focused->focused = false;
				new_top = w;
				focused = w;
			}
		}
	}

	// Moves new_top to the back (front of screen)
	if(new_top != nullptr)
	{
		windows.remove(new_top);
		windows.push_back(new_top);
	}
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

	// Prevent click through to other GUIs and to other game features
	gui_input->ext_mouse_blocked = any_hovered || original_block_mouse;
	gui_input->ext_keyboard_blocked = any_hovered || original_block_keyboard;
	gui_input->mouse_blocked = any_hovered || gui_input->mouse_blocked;
	gui_input->keyboard_blocked = any_hovered || gui_input->keyboard_blocked;
}

void GUIWindowManager::draw(NVGcontext* vg, GUISkin* skin)
{
	for(GUIWindow* w : windows)
	{
		w->draw(vg, skin, viewport);
	}

}

GUIWindow* GUIWindowManager::create_window()
{
	GUIWindow* n_window = new GUIWindow();

	n_window->focused = false;

	windows.push_front(n_window);
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

GUIWindowManager::GUIWindowManager() 
{
	windows = std::list<GUIWindow*>();
	focused = nullptr;
	
}
