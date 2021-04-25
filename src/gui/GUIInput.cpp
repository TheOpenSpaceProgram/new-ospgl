#include "GUIInput.h"
#include <util/InputUtil.h>


bool GUIInput::mouse_inside(glm::ivec2 pos, glm::ivec2 size)
{
	glm::ivec2 mpos = glm::ivec2(input->mouse_pos);

	if(mpos.x >= pos.x && mpos.y >= pos.y && 
		mpos.x < pos.x + size.x && mpos.y <= pos.y + size.y)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool GUIInput::mouse_inside(glm::ivec4 aabb) 
{
	return mouse_inside(glm::ivec2(aabb.x, aabb.y), glm::ivec2(aabb.z, aabb.w)); 
}

bool GUIInput::mouse_int(int btn)
{
	if(btn < 0 || btn > 2)
	{
		return false;
	}

	if(block_mouse[btn])
	{
		return false;
	}

	return true;
}

bool GUIInput::mouse_pressed(int btn)
{
	return mouse_int(btn) && input->mouse_pressed(btn);	
}

bool GUIInput::mouse_down(int btn)
{
	return mouse_int(btn) && input->mouse_down(btn);
}

bool GUIInput::mouse_up(int btn)
{
	return mouse_int(btn) && input->mouse_up(btn);
}

void GUIInput::update()
{
	mouse_blocked = false;
	keyboard_blocked = false;
	scroll_blocked = false;
	ext_mouse_blocked = false;
	ext_keyboard_blocked = false;
	ext_scroll_blocked = false;
	
	for(int i = 0; i < 3; i++)
	{
		if(input->mouse_up(i))
		{
			block_mouse[i] = false;
		}
	}
}

double GUIInput::mouse_scroll_delta() const
{
	if(scroll_blocked || ext_scroll_blocked)
	{
		return 0.0;
	}

	return input->mouse_scroll_delta;

}
