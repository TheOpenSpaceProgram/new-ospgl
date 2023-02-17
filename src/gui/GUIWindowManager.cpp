#include "GUIWindowManager.h"
#include <util/InputUtil.h>
#include <gui/GUIScreen.h>

void GUIWindowManager::position(GUIScreen *screen)
{
	// Order doesn't really matter for position as windows are independent
	for(auto w : windows)
	{
		w->position(screen, screen->skin.get());
	}
}

void GUIWindowManager::pre_prepare(GUIScreen *screen)
{
	for(auto w : windows)
	{
		w->pre_prepare(screen);
	}
}

void GUIWindowManager::prepare(GUIInput* gui_input, GUIScreen* screen)
{
	GUISkin* skin = screen->skin.get();
	if(windows.size() == 0)
	{
		return;
	}

	bool any_down = gui_input->mouse_down(0) || gui_input->mouse_down(1) || gui_input->mouse_down(2);
	bool left_down = gui_input->mouse_down(0);

	bool found_focus = false;
	std::shared_ptr<GUIWindow> new_top = nullptr;

	bool any_hovered = false;

	if(!gui_input->mouse_blocked && !gui_input->ext_mouse_blocked)
	{
		if (dragging)
		{
			any_hovered = true;
			focused->next_pos = glm::vec2(input->mouse_pos) - drag_point;
			if (gui_input->mouse_up(0))
			{
				dragging = false;
			}
		}
		else if (resizing)
		{
			any_hovered = true;
			glm::ivec2 delta_mouse = glm::ivec2(input->mouse_pos) - glm::ivec2(drag_point);
			glm::vec2 old_size = focused->size;
			glm::vec2 old_pos = focused->pos;

			if (resize_point == GUISkin::BOTTOM)
			{
				focused->next_size.y = size_0.y + delta_mouse.y;
			}
			else if (resize_point == GUISkin::BOTTOM_RIGHT)
			{
				focused->next_size = size_0 + delta_mouse;
			}
			else if (resize_point == GUISkin::RIGHT)
			{
				focused->next_size.x = size_0.x + delta_mouse.x;
			}
			else if (resize_point == GUISkin::TOP_RIGHT)
			{
				focused->next_size.x = size_0.x + delta_mouse.x;
				focused->next_size.y = size_0.y - delta_mouse.y;
				focused->next_pos.y = pos_0.y + delta_mouse.y;
			}
			else if (resize_point == GUISkin::TOP)
			{
				focused->next_size.y = size_0.y - delta_mouse.y;
				focused->next_pos.y = pos_0.y + delta_mouse.y;
			}
			else if (resize_point == GUISkin::TOP_LEFT)
			{
				focused->next_size = size_0 - delta_mouse;
				focused->next_pos = pos_0 + delta_mouse;
			}
			else if (resize_point == GUISkin::LEFT)
			{
				focused->next_size.x = size_0.x - delta_mouse.x;
				focused->next_pos.x = pos_0.x + delta_mouse.x;
			}
			else if (resize_point == GUISkin::BOTTOM_LEFT)
			{
				focused->next_size.x = size_0.x - delta_mouse.x;
				focused->next_size.y = size_0.y + delta_mouse.y;
				focused->next_pos.x = pos_0.x + delta_mouse.x;
			}

			if (focused->next_size.x < focused->min_size.x)
			{
				focused->next_pos.x = old_pos.x;
				focused->next_size.x = old_size.x;
			}

			if (focused->next_size.y < focused->min_size.y)
			{
				focused->next_pos.y = old_pos.y;
				focused->next_size.y = old_size.y;
			}

			if (gui_input->mouse_up(0))
			{
				resizing = false;
			}
		}
		else
		{
			bool remove = false;

			// We iterate in reverse (painter's algorithm is used for rendering)
			for (auto it = windows.rbegin(); it != windows.rend(); it++)
			{
				glm::ivec2 mouse_pos = input->mouse_pos;
				auto w = *it;

				w->close_hovered = false;
				w->minimize_hovered = false;
				w->pin_hovered = false;
				w->drag_hovered = false;

				bool mouse_inside = false;
				glm::ivec4 aabb = skin->get_window_aabb(w.get());
				if (gui_input->mouse_inside(glm::vec2(aabb.x, aabb.y), glm::vec2(aabb.z, aabb.w)) && !found_focus)
				{
					any_hovered = true;
					mouse_inside = true;
					GUISkin::ResizePoint rpt;
					if (w->resizeable)
					{
						rpt = skin->can_resize_window(w.get(), mouse_pos);
					}
					else
					{
						rpt = GUISkin::NONE;
					}

					if (w->closeable && skin->can_close_window(w.get(), mouse_pos))
					{
						w->close_hovered = true;
						if (gui_input->mouse_down(0) && gui_input->execute_user_actions)
						{
							remove = true;
							w->open = false;
							w->emit_event("on_close");
						}
					}
					else if (w->minimizable && skin->can_minimize_window(w.get(), mouse_pos))
					{
						w->minimize_hovered = true;
						if (gui_input->mouse_down(0) && gui_input->execute_user_actions)
						{
							w->minimized = !w->minimized;
						}
					}
					else if (w->pinable && skin->can_pin_window(w.get(), mouse_pos))
					{
						w->pin_hovered = true;

						if (gui_input->mouse_down(0) && gui_input->execute_user_actions)
						{
							w->pinned = !w->pinned;
						}
					}
					else if (rpt != GUISkin::NONE)
					{
						if (rpt == GUISkin::TOP || rpt == GUISkin::BOTTOM)
						{
							input->set_cursor(InputUtil::Cursor::RESIZE_NS);
						}
						else if (rpt == GUISkin::LEFT || rpt == GUISkin::RIGHT)
						{
							input->set_cursor(InputUtil::Cursor::RESIZE_EW);
						}
						else if (rpt == GUISkin::BOTTOM_LEFT || rpt == GUISkin::TOP_RIGHT)
						{
							input->set_cursor(InputUtil::Cursor::RESIZE_NESW);
						}
						else if (rpt == GUISkin::TOP_LEFT || rpt == GUISkin::BOTTOM_RIGHT)
						{
							input->set_cursor(InputUtil::Cursor::RESIZE_NWSE);
						}

						if (left_down && gui_input->execute_user_actions)
						{
							resizing = true;
							resize_point = rpt;
							drag_point = mouse_pos;
							pos_0 = w->pos;
							size_0 = w->size;
						}
					}
					else if (w->moveable && skin->can_drag_window(w.get(), mouse_pos))
					{
						w->drag_hovered = true;
						if (left_down && gui_input->execute_user_actions)
						{
							dragging = true;
							drag_point = glm::vec2(mouse_pos - w->pos);
						}
					}
					found_focus = true;
					if (w != focused && any_down && gui_input->execute_user_actions)
					{
						w->focused = true;
						if(focused)
							focused->focused = false;
						new_top = w;
						focused = w;
					}
				}
			}

			if (remove)
			{
				windows.remove(focused);
				new_top = nullptr;
			}
		}


		// Moves new_top to the back (front of screen)
		if (new_top != nullptr)
		{
			windows.remove(new_top);
			windows.push_back(new_top);
		}
	}
	bool original_block_mouse = gui_input->ext_mouse_blocked;
	bool original_block_keyboard = gui_input->ext_keyboard_blocked;
	bool original_block_scroll = gui_input->ext_scroll_blocked;

	// Reverse order so top-windows block bottom ones
	for(auto it = windows.rbegin(); it != windows.rend(); it++)
	{
		auto w = *it;
		bool old_mouse_block = gui_input->ext_mouse_blocked;
		bool old_keyboard_block = gui_input->ext_keyboard_blocked;
		bool old_scroll_block = gui_input->ext_scroll_blocked;
		if(w != focused)
		{
			// Block user input for non focused windows
			gui_input->ext_mouse_blocked = true;
			gui_input->ext_keyboard_blocked = true;
			gui_input->ext_scroll_blocked = true;
		}
		w->prepare(gui_input, screen);

		gui_input->ext_mouse_blocked = old_mouse_block;
		gui_input->ext_keyboard_blocked = old_keyboard_block;
		gui_input->ext_scroll_blocked = old_scroll_block;
	}

	// Prevent click through to other GUIs and to other game features
	gui_input->mouse_blocked = any_hovered || gui_input->mouse_blocked;
	gui_input->keyboard_blocked = any_hovered || gui_input->keyboard_blocked;
	gui_input->scroll_blocked = any_hovered || gui_input->scroll_blocked;
}

void GUIWindowManager::draw(NVGcontext* vg, GUIScreen* screen)
{
	for(auto w : windows)
	{
		w->draw(vg, screen->skin.get(), viewport);
	}

}

std::weak_ptr<GUIWindow> GUIWindowManager::create_window(glm::ivec2 pos, glm::ivec2 size)
{
	auto n_window = std::make_shared<GUIWindow>();
	if(pos.x < 0)
		pos.x = default_pos.x;
	if(pos.y < 0)
		pos.y = default_pos.y;
	if(size.x < 0)
		size.x = default_size.x;
	if(size.y < 0)
		size.y = default_size.y;

	n_window->pos = pos;
	n_window->next_pos = pos;
	n_window->size = size;
	n_window->next_size = size;
	n_window->open = true;
	n_window->focused = false;
	n_window->wman = this;

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
	for(auto it = windows.begin(); it != windows.end(); it++)
	{
		if(it->get() == win)
		{
			windows.erase(it);
			break;
		}
	}

	win->open = false;
	win->emit_event("on_close");
	if(focused.get() == win)
	{
		focused = nullptr;
	}
}

GUIWindowManager::GUIWindowManager() 
{
	windows = std::list<std::shared_ptr<GUIWindow>>();
	focused = nullptr;
	dragging = false;
	resizing = false;

	default_pos = glm::ivec2(400, 100);
	default_size = glm::ivec2(400, 400);
}

