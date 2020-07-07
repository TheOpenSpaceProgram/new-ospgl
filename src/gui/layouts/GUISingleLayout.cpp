#include "GUISingleLayout.h"


void GUISingleLayout::prepare(GUIInput* gui_input)
{
	// We simply draw the first widget, everything else is ignored
	if(widgets.size() == 0)
	{
		return;
	}	

	glm::ivec2 pos = get_pos();
	glm::ivec2 size = get_size();

	pos += glm::ivec2(margins.x, margins.z);
	size -= glm::ivec2(margins.y, margins.w) * 2; //< *2 because left and right are taken

	widgets[0]->is_visible = true;
	widgets[0]->prepare(glm::ivec2(pos.x, pos.y), glm::ivec2(size.x, size.y), gui_input);

	vscrollbar.draw = false;
}
