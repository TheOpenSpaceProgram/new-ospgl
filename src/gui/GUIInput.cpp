#include "GUIInput.h"
#include <util/InputUtil.h>


bool GUIInput::is_mouse_inside(glm::ivec2 pos, glm::ivec2 size)
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


bool GUIInput::is_mouse_clicked(int btn)
{
	return glfwGetMouseButton(input->window, btn) == GLFW_PRESS; 
}
