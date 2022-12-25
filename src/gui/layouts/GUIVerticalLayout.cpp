#include "GUIVerticalLayout.h"

#include <util/Logger.h>

void GUIVerticalLayout::position(glm::ivec2 vpos, glm::ivec2 vsize, GUIScreen *screen)
{
	this->pos = vpos;
	this->size = vsize;

	vpos += glm::ivec2(margins.x, margins.z);
	vsize -= glm::ivec2(margins.y, margins.w) * 2;

	int y_pos = pos.y - vscrollbar.scroll;
	for(auto widget : widgets)
	{
		// Y size is free
		glm::ivec2 used = widget->position(glm::ivec2(pos.x, y_pos), glm::ivec2(size.x, -1.0),
										   screen);

		// Culling
		if(y_pos - pos.y > size.y || y_pos - pos.y < -used.y)
		{
			widget->is_visible = false;
		}
		else
		{
			widget->is_visible = true;
		}

		y_pos += used.y + element_margin;

	}

	vscrollbar.max_scroll = y_pos - pos.y + vscrollbar.scroll;

}

void GUIVerticalLayout::prepare(GUIInput* gui_input, GUIScreen* screen)
{
	for(auto widget : widgets)
	{
		widget->prepare(get_aabb(), screen, gui_input);
	}

}

void GUIVerticalLayout::pre_prepare(GUIScreen *screen)
{
	for(auto widget : widgets)
	{
		widget->pre_prepare(screen);
	}
}
