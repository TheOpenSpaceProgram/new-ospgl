#pragma once
#include <glm/glm.hpp>
#define GLFW_INCLUDE_NONE 
#include <GLFW/glfw3.h>
#include <cstring>

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// Central class for making GLFW inputs easy to handle
class InputUtil
{
private:



public:

	glm::dvec2 prev_mouse_pos;
	glm::dvec2 mouse_pos;
	glm::dvec2 mouse_delta;

	double mouse_scroll_delta;
	double prev_mouse_scroll;
	double mouse_scroll;

	bool current_key_status[GLFW_KEY_LAST] = {false};
	bool prev_key_status[GLFW_KEY_LAST] = {false};

	GLFWwindow* window;

	void setup(GLFWwindow* window)
	{
		glfwSetScrollCallback(window, scroll_callback);
		glfwSetKeyCallback(window, key_callback);
		this->window = window;
	}

	void update(GLFWwindow* window)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		prev_mouse_pos = mouse_pos;
		mouse_pos = glm::dvec2(xpos, ypos);
		mouse_delta = mouse_pos - prev_mouse_pos;

		mouse_scroll_delta = mouse_scroll - prev_mouse_scroll;
		mouse_scroll = 0.0;
		prev_mouse_scroll = mouse_scroll;

		std::memcpy(prev_key_status, current_key_status, GLFW_KEY_LAST * sizeof(bool));
		for(int i = 0; i < GLFW_KEY_LAST; i++)
		{
			current_key_status[i] = false;
		}
	}

	bool key_pressed(int key)
	{
		return current_key_status[key];	
	}

	bool key_down(int key)
	{
		return current_key_status[key] && !prev_key_status[key];
	}

	bool key_up(int key)
	{
		return !current_key_status[key] && prev_key_status[key];
	}
};

extern InputUtil* input;

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	input->mouse_scroll = yoffset;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	input->current_key_status[key] = action == GLFW_PRESS;	/*|| action == GLFW_REPEAT;*/	
}
