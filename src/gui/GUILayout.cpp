#include "GUILayout.h"
#include <util/Logger.h>

void GUILayout::add_widget(std::shared_ptr<GUIWidget> widget)
{
	widgets.push_back(widget);
	on_add_widget(widget.get());
}

void GUILayout::remove_widget(GUIWidget* widget)
{
	for(auto it = widgets.begin(); it != widgets.end(); it++)
	{
		if(it->get() == widget)
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


	int count = 0;
	for(auto widget : widgets)
	{
		if(widget->is_visible)
		{
			nvgScissor(ctx, pos.x, pos.y, size.x, size.y);
			widget->draw(ctx, skin);
			count++;
		}
	}

	draw_vscrollbar(ctx, skin);
	draw_hscrollbar(ctx, skin);
}

void GUILayout::position_wrapper(glm::ivec2 npos, glm::ivec2 nsize, GUIScreen* screen)
{
	pos = npos;
	size = nsize;

	position(pos, size, screen);
}

void GUILayout::prepare_wrapper(GUIScreen* screen, GUIInput* gui_input)
{
	// Prepare is top-to-bottom so first the widgets, then scrollbar and finally the layout itself
	prepare(gui_input, screen);

	prepare_scrollbar(gui_input, &vscrollbar, true);
	prepare_scrollbar(gui_input, &hscrollbar, false);

	if(gui_input->mouse_inside(get_pos(), get_size()) && block_mouse)
	{
		gui_input->mouse_blocked = true;
	}

}

void GUILayout::remove_all_widgets()
{
	for(std::shared_ptr<GUIWidget> w : widgets)
	{
		on_remove_widget(w.get());
	}

	widgets.clear();
}

#include <util/InputUtil.h>

void GUILayout::prepare_scrollbar(GUIInput *gui_input, GUIScrollbar *scroll, bool v)
{
	if(!scroll->enabled)
	{
		scroll->draw = false;
		return;
	}

	if(scroll->enabled)
	{
		if(gui_input->mouse_inside(get_pos(), get_size()) && !gui_input->ext_scroll_blocked)
		{
			scroll->scroll -= input->mouse_scroll * 40.0;

			float max;
			float mix;
			if(v)
			{
				max = size.y + margins.w;
				mix = size.y - margins.w;
			}
			else
			{
				max = size.x + margins.z;
				mix = size.x - margins.z;
			}
			if(scroll->max_scroll <= max)
			{
				scroll->scroll = 0;
			}
			else
			{
				if(scroll->scroll <= 0)
				{
					scroll->scroll = 0;
				}

				if(scroll->scroll >= scroll->max_scroll - mix && scroll->scroll > 0)
				{
					scroll->scroll = scroll->max_scroll - mix;
				}
			}
		}
	}
	else
	{
		scroll->scroll = 0.0;
	}
}


void GUILayout::draw_hscrollbar(NVGcontext *vg, GUISkin* skin)
{
	if(!hscrollbar.draw)
		return;

	float y = 0.0f;
	if(hscrollbar.positive_pos)
		y = (float)size.y - (float)hscrollbar.get_width(skin);

	nvgBeginPath(vg);
	nvgRect(vg, pos.x, y + pos.y, size.x, (float)hscrollbar.get_width(skin));
	nvgFillColor(vg, hscrollbar.get_color(skin));
	nvgFill(vg);
	float scroll_w = (float)size.x * (float)size.x / (float)hscrollbar.max_scroll;
	float scroll_x = (float)(size.x - scroll_w) * (float)hscrollbar.scroll / ((float)hscrollbar.max_scroll - size.x);
	if(scroll_w > size.x)
		scroll_w = size.x;

	nvgBeginPath(vg);
	nvgRect(vg, pos.x + scroll_x, pos.y + y, scroll_w, (float)hscrollbar.get_width(skin));
	nvgFillColor(vg, hscrollbar.get_scroller_color(skin));
	nvgFill(vg);
}

void GUILayout::draw_vscrollbar(NVGcontext* vg, GUISkin* skin)
{
	if(!vscrollbar.draw)
		return;
	float x = 0.0f;
	if(vscrollbar.positive_pos)
		x = (float)size.x - (float)vscrollbar.get_width(skin);

	nvgBeginPath(vg);
	nvgRect(vg, x + pos.x, pos.y, (float)vscrollbar.get_width(skin), (float)size.y);
	nvgFillColor(vg, vscrollbar.get_color(skin));
	nvgFill(vg);
	float scroll_h = (float)size.y * (float)size.y / (float)vscrollbar.max_scroll;
	float scroll_y = (float)(size.y - scroll_h) * (float)vscrollbar.scroll / ((float)vscrollbar.max_scroll - size.y);
	if(scroll_h > size.y)
		scroll_h = size.y;

	nvgBeginPath(vg);
	nvgRect(vg, x + pos.x, pos.y + scroll_y, (float)vscrollbar.get_width(skin), scroll_h);
	nvgFillColor(vg, vscrollbar.get_scroller_color(skin));
	nvgFill(vg);
}


GUILayout::GUILayout()
{
	// Disable scrollbar
	vscrollbar.enabled = false;
	vscrollbar.override_width = false;
	vscrollbar.override_colors = false;
	vscrollbar.scroll = 0.0;
	hscrollbar.enabled = false;
	hscrollbar.override_width = false;
	hscrollbar.override_colors = false;
	hscrollbar.scroll = 0.0;
	block_mouse = true;
}

GUILayout::~GUILayout()
{
}

