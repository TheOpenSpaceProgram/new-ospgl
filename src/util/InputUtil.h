#pragma once
#include <glm/glm.hpp>

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

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

	void setup(GLFWwindow* window)
	{
		glfwSetScrollCallback(window, scroll_callback);
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

	}
};

extern InputUtil* input;

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	input->mouse_scroll = yoffset;
}