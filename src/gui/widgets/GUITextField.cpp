#include "GUITextField.h"

glm::ivec2 GUITextField::prepare(glm::ivec2 wpos, glm::ivec2 wsize, GUIInput* gui_input) 
{
	pos = wpos;
	size = wsize;
	if(size.x < 0)
	{
		size.x = wsize.x;
	}

	if(size.y < 0)
	{
		size.y = wsize.y;
	}

	return size;
}

void GUITextField::draw(NVGcontext* vg, GUISkin* skin) 
{	
	// TODO: Maybe write a new skin function for text fields?
	skin->draw_button(vg, pos, size, "", GUISkin::ButtonState::NORMAL, GUISkin::ButtonStyle::SYMMETRIC);
}
