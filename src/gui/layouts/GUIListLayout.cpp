#include "GUIListLayout.h"

void GUIListLayout::prepare(GUIInput* gui_input)
{
	// We lay the widgets left to right, if a widget overflows
	// horizontally, we move onto the next line (by preparing
	// again the widget)
	
	glm::ivec2 pos = get_pos();
	glm::ivec2 size = get_size();

	pos += glm::ivec2(margins.x, margins.z);
	size -= glm::ivec2(margins.y, margins.w) * 2; //< *2 because left and right are taken

	// Simply, we lay the widgets increasing only the y position
	// We ignore maximum size in Y, we will use a scrollbar if needed
	
	int y_pos = pos.y - vscrollbar.scroll;
	int x_pos = pos.x;
	for(auto widget : widgets)
	{
		// Y size is free
		glm::ivec2 used = widget->prepare(glm::ivec2(x_pos, y_pos), glm::ivec2(size.x, -1.0), get_aabb(), gui_input);

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
