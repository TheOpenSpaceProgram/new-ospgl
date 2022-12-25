#include "GUIListLayout.h"

void GUIListLayout::position(glm::ivec2 vpos, glm::ivec2 vsize, GUIScreen *screen)
{
	pos = vpos;
	size = vsize;

	vpos += glm::ivec2(margins.x, margins.z);
	vsize -= glm::ivec2(margins.y, margins.w) * 2; //< *2 because left and right are taken

	int y_pos = pos.y - vscrollbar.scroll;
	int x_pos = pos.x;
	for(auto widget : widgets)
	{
		// Y size is free
		glm::ivec2 used = widget->position(glm::ivec2(x_pos, y_pos), glm::ivec2(size.x, -1.0),
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

		x_pos += used.x + element_hmargin;

		// Check for horizontal overflow, if we touch the margin we are out
		if(x_pos - pos.x + used.x > size.x)
		{
			y_pos += used.y + element_vmargin;
			x_pos = pos.x;
		}
	}

	vscrollbar.max_scroll = y_pos - pos.y + vscrollbar.scroll;
}

void GUIListLayout::prepare(GUIInput* gui_input, GUIScreen* screen)
{
	for(auto widget : widgets)
	{
		widget->prepare(get_aabb(), screen, gui_input);
	}
}

void GUIListLayout::pre_prepare(GUIScreen *screen)
{
	for(auto widget : widgets)
	{
		widget->pre_prepare(screen);
	}
}
