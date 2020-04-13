#include "JoystickDebug.h"
#include <imgui/imgui.h>
#include <GLFW/glfw3.h>
#include <string>

void JoystickDebug::do_imgui()
{
	ImGui::Begin("Joystick Debug");

	for(int i = 0; i < 16; i++)
	{
		if(glfwJoystickPresent(i))
		{
			std::string joy_name = glfwGetJoystickName(i);
			std::string name_str = "Joystick " + std::to_string(i) + " - " + joy_name;
			if(ImGui::CollapsingHeader(name_str.c_str()))
			{
				int count;
				const float* axes = glfwGetJoystickAxes(i, &count);

				for(int j = 0; j < count; j++)
				{
					ImGui::Text("Axis %i: %f", j, axes[j]);	
				}

				const unsigned char* buttons = glfwGetJoystickButtons(i, &count);

				for(int j = 0; j < count; j++)
				{
					int pressed = buttons[j] == GLFW_PRESS;
					ImGui::Text("Button %i: %i", j, pressed);
				}
			}
		}
	}

	ImGui::End();
}
