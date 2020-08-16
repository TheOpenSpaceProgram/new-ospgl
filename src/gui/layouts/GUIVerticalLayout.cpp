#include "GUIVerticalLayout.h"

#include <util/Logger.h>

void GUIVerticalLayout::prepare(GUIInput* gui_input)
{
	glm::ivec2 pos = get_pos();
	glm::ivec2 size = get_size();

	pos += glm::ivec2(margins.x, margins.z);
	size -= glm::ivec2(margins.y, margins.w) * 2; //< *2 because left and right are taken

	// Simply, we lay the widgets increasing only the y position
	// We ignore maximum size in Y, we will use a scrollbar if needed
	
	int y_pos = pos.y - vscrollbar.scroll;
	for(auto widget : widgets)
	{
		// Y size is free
		glm::ivec2 used = widget->prepare(glm::ivec2(pos.x, y_pos), glm::ivec2(size.x, -1.0), get_aabb(), gui_input);

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
