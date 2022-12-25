#include "GUIInput.h"
#include <util/InputUtil.h>
#include <imgui/imgui.h>


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
	execute_user_actions = true;
	
	for(int i = 0; i < 3; i++)
	{
		if(input->mouse_up(i))
		{
			block_mouse[i] = false;
		}
	}
}

void GUIInput::debug()
{
	ImGui::Begin("GUI Input debug");

	ImGui::Text("ext_mouse_blocked = %i", ext_mouse_blocked);
	ImGui::Text("ext_scroll_blocked = %i", ext_scroll_blocked);
	ImGui::Text("ext_keyboard_blocked = %i", ext_keyboard_blocked);
	ImGui::Separator();
	ImGui::Text("mouse_blocked = %i", mouse_blocked);
	ImGui::Text("scroll_blocked = %i", scroll_blocked);
	ImGui::Text("keyboard_blocked = %i", keyboard_blocked);
	ImGui::Separator();
	ImGui::Text("execute_user_actions = %i", execute_user_actions);
	ImGui::Text("block_mouse[0] = %i", block_mouse[0]);
	ImGui::Text("block_mouse[1] = %i", block_mouse[1]);
	ImGui::Text("block_mouse[2] = %i", block_mouse[2]);

	ImGui::End();

}
