#include "GUILayout.h"
#include <util/Logger.h>

void GUILayout::add_widget(GUIWidget* widget)
{
	widgets.push_back(widget);

	on_add_widget(widget);
}

void GUILayout::remove_widget(GUIWidget* widget)
{
	for(auto it = widgets.begin(); it != widgets.end(); it++)
	{
		if(*it == widget)
		{
			on_remove_widget(widget);
			widgets.erase(it);
			return;
		}
	}

	logger->warn("Tried to remove a widget which was not present");
}

size_t GUILayout::get_widget_count()
{
	return widgets.size();
}

void GUILayout::draw(NVGcontext* ctx, GUISkin* skin)
{
	glm::ivec2 pos = get_pos();
	glm::ivec2 size = get_size();

	nvgScissor(ctx, pos.x, pos.y, size.x, size.y);	

	int count = 0;
	for(auto widget : widgets)
	{
		if(widget->is_visible)
		{
			widget->draw(ctx, skin);
			count++;
		}
	}

	draw_vscrollbar(ctx);
}

void GUILayout::prepare_wrapper(glm::ivec2 pos, glm::ivec2 size, GUIInput* gui_input)
{
	this->pos = pos;
	this->size = size;

	if(gui_input->mouse_inside(get_pos(), get_size()) && block_mouse)
	{
		gui_input->mouse_blocked = true;
	}

	prepare(gui_input);

	prepare_vscrollbar(gui_input);
}

void GUILayout::remove_all_widgets()
{
	for(GUIWidget* w : widgets)
	{
		on_remove_widget(w);
	}

	widgets.clear();
}

#include <util/InputUtil.h>

void GUILayout::prepare_vscrollbar(GUIInput* gui_input)
{
	if(vscrollbar.enabled)
	{
		if(gui_input->mouse_inside(get_pos(), get_size()))
		{
			vscrollbar.scroll -= input->mouse_scroll * 40.0;

			if(vscrollbar.max_scroll <= size.y + margins.w)
			{
				vscrollbar.scroll = 0;
			}
			else 
			{
				if(vscrollbar.scroll <= 0)
				{
					vscrollbar.scroll = 0;
				}

				if(vscrollbar.scroll >= vscrollbar.max_scroll - size.y + margins.w && vscrollbar.scroll > 0)
				{
					vscrollbar.scroll = vscrollbar.max_scroll - size.y + margins.w;
				}
			}
		}
	}
	else
	{
		vscrollbar.scroll = 0.0;
	}
}

void GUILayout::draw_vscrollbar(NVGcontext* vg)
{
	if(vscrollbar.draw)
	{
		glm::ivec2 pos = get_pos();
		glm::ivec2 size = get_size();

		float x = 0.0f;
		if(vscrollbar.positive_pos)
		{
			x = (float)size.x - (float)vscrollbar.width;
		}

		nvgBeginPath(vg);
		nvgRect(vg, x + pos.x, pos.y, (float)vscrollbar.width, (float)size.y);
		nvgFillColor(vg, vscrollbar.color);
		nvgFill(vg);
		float scroll_h = (float)size.y * (float)size.y / (float)vscrollbar.max_scroll;
		float scroll_y = (float)(size.y - scroll_h) * (float)vscrollbar.scroll / ((float)vscrollbar.max_scroll - size.y);

		nvgBeginPath(vg);
		nvgRect(vg, x + pos.x, pos.y + scroll_y, (float)vscrollbar.width, scroll_h);
		nvgFillColor(vg, vscrollbar.scroller_color);
		nvgFill(vg);
	}
}


GUILayout::GUILayout()
{
	// Disable scrollbar
	vscrollbar.enabled = false;
	block_mouse = true;
}

GUILayout::~GUILayout()
{
	for(GUIWidget* w : widgets)
	{
		delete w;
	}
}
