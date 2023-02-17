#include "GUIHorizontalLayout.h"

#include <util/Logger.h>
#include <gui/GUIScreen.h>

void GUIHorizontalLayout::position(glm::ivec2 vpos, glm::ivec2 vsize, GUIScreen *screen)
{
	this->pos = vpos;
	this->size = vsize;

	vpos += glm::ivec2(margins.x, margins.z);
	vsize -= glm::ivec2(margins.x + margins.y, margins.z + margins.w);
	vsize.y -= hscrollbar.get_width(screen->skin.get());

	int x_pos = vpos.x - hscrollbar.scroll;
	for(auto widget : widgets)
	{
		// X size is free
		glm::ivec2 used = widget->position(glm::ivec2(x_pos, vpos.y), glm::ivec2(-1.0, vsize.y),
										   screen);

		// Culling
		if (x_pos - vpos.x > vsize.x || x_pos - vpos.x < -used.x)
		{
			widget->is_visible = false;
		}
		else
		{
			widget->is_visible = true;
		}

		x_pos += used.x + element_margin;
	}

	hscrollbar.max_scroll = x_pos - vpos.x + hscrollbar.scroll;

}

void GUIHorizontalLayout::prepare(GUIInput* gui_input, GUIScreen* screen)
{
	for(auto widget : widgets)
	{
		widget->prepare(get_aabb(), screen, gui_input);
	}

	if(gui_input->mouse_inside(get_aabb()))
	{
		gui_input->scroll_blocked = true;
	}

}

void GUIHorizontalLayout::pre_prepare(GUIScreen *screen)
{
	for(auto widget : widgets)
	{
		widget->pre_prepare(screen);
	}
}
