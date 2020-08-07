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


	if(dragging)
	{
		any_hovered = true;
		focused->pos = glm::vec2(input->mouse_pos) - drag_point;
		if(gui_input->mouse_up(0))
		{
			dragging = false;
		}
	}
	else if(resizing)
	{
		any_hovered = true;
		glm::ivec2 delta_mouse = glm::ivec2(input->mouse_pos) - glm::ivec2(drag_point);
		glm::vec2 old_size = focused->size;
		glm::vec2 old_pos = focused->pos;

		if(resize_point == GUISkin::BOTTOM)
		{
			focused->size.y = size_0.y + delta_mouse.y;
		}
		else if(resize_point == GUISkin::BOTTOM_RIGHT)
		{
			focused->size = size_0 + delta_mouse;
		}
		else if(resize_point == GUISkin::RIGHT)
		{
			focused->size.x = size_0.x + delta_mouse.x;
		}
		else if(resize_point == GUISkin::TOP_RIGHT)
		{
			focused->size.x = size_0.x + delta_mouse.x;
			focused->size.y = size_0.y - delta_mouse.y;
			focused->pos.y = pos_0.y + delta_mouse.y;
		}
		else if(resize_point == GUISkin::TOP)
		{
			focused->size.y = size_0.y - delta_mouse.y;
			focused->pos.y = pos_0.y + delta_mouse.y;
		}
		else if(resize_point == GUISkin::TOP_LEFT)
		{
			focused->size = size_0 - delta_mouse;
			focused->pos = pos_0 + delta_mouse;
		}
		else if(resize_point == GUISkin::LEFT)
		{
			focused->size.x = size_0.x - delta_mouse.x;
			focused->pos.x = pos_0.x + delta_mouse.x;
		}
		else if(resize_point == GUISkin::BOTTOM_LEFT)
		{
			focused->size.x = size_0.x - delta_mouse.x;
			focused->size.y = size_0.y + delta_mouse.y;
			focused->pos.x = pos_0.x + delta_mouse.x;
		}

		if(focused->size.x < focused->min_size.x)
		{
			focused->pos.x = old_pos.x;
			focused->size.x = old_size.x;
		}

		if(focused->size.y < focused->min_size.y)
		{
			focused->pos.y = old_pos.y;
			focused->size.y = old_size.y;
		}

		if(gui_input->mouse_up(0))
		{
			resizing = false;
		}
	}
	else
	{
		// We iterate in reverse (painter's algorithm is used for rendering)
		for(auto it = windows.rbegin(); it != windows.rend(); it++)
		{
			glm::ivec2 mouse_pos = input->mouse_pos;
			GUIWindow* w = *it;

			w->close_hovered = false; w->minimize_hovered = false;
			w->pin_hovered = false; w->drag_hovered = false;

			bool mouse_inside = false;
			glm::ivec4 aabb = skin->get_window_aabb(w);
			if(gui_input->mouse_inside(glm::vec2(aabb.x, aabb.y), glm::vec2(aabb.z, aabb.w)) && !found_focus)
			{
				any_hovered = true;
				mouse_inside = true;
				GUISkin::ResizePoint rpt;
				if(w->resizeable)
				{
					rpt = skin->can_resize_window(w, mouse_pos);
				}
				else
				{
					rpt = GUISkin::NONE;
				}

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
				else if(rpt != GUISkin::NONE)
				{
					if(rpt == GUISkin::TOP || rpt == GUISkin::BOTTOM)
					{
						input->set_cursor(InputUtil::Cursor::RESIZE_NS);
					}
					else if(rpt == GUISkin::LEFT || rpt == GUISkin::RIGHT)
					{
						input->set_cursor(InputUtil::Cursor::RESIZE_EW);
					}
					else
					{
						input->set_cursor(InputUtil::Cursor::RESIZE_ALL);
					}
					
					if(mouse_down)
					{
						resizing = true;
						resize_point = rpt;
						drag_point = mouse_pos;
						pos_0 = w->pos;
						size_0 = w->size;
					}	
				}
				else if(w->moveable && skin->can_drag_window(w, mouse_pos))
				{
					w->drag_hovered = true;
					if(mouse_down)
					{
						dragging = true;
						drag_point = glm::vec2(mouse_pos - w->pos);
					}
				}
				found_focus = true;
				if(w != focused && mouse_down)
				{
					w->focused = true;
					focused->focused = false;
					new_top = w;
					focused = w;
				}
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
	dragging = false;
	resizing = false;
	
}
