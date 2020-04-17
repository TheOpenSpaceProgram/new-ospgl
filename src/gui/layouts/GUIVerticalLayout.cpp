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
	
	int y_pos = pos.y;
	for(auto widget : widgets)
	{
		// Y size is free
		glm::ivec2 used = widget->prepare(glm::ivec2(pos.x, y_pos), glm::ivec2(size.x, -1.0), gui_input);
		y_pos += used.y + element_margin;
	}

}
