#pragma once
#include <glm/glm.hpp>
#define GLFW_INCLUDE_NONE 
#include <GLFW/glfw3.h>
#include <cstring>

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


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

	static const size_t KEY_LIST_SIZE = GLFW_KEY_LAST + GLFW_MOUSE_BUTTON_LAST;

	bool current_key_status[KEY_LIST_SIZE] = {false};
	bool prev_key_status[KEY_LIST_SIZE] = {false};

	GLFWwindow* window;

	void setup(GLFWwindow* window)
	{
		glfwSetScrollCallback(window, scroll_callback);
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

		std::memcpy(prev_key_status, current_key_status, KEY_LIST_SIZE * sizeof(bool));
		for(int i = 0; i < KEY_LIST_SIZE; i++)
		{
			current_key_status[i] = false;
		}

		auto check_key = [this](int k)
		{
			this->current_key_status[k] = glfwGetKey(this->window, k);
		};

		auto check_mouse = [this](int k)
		{
			this->current_key_status[k + GLFW_KEY_LAST] = glfwGetMouseButton(this->window, k);
		}; 

		
		for(int i = 0; i < GLFW_KEY_LAST; i++)
		{
			check_key(i);
		}

		for(int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
		{
			check_mouse(i);
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

	bool mouse_pressed(int button)
	{
		return key_pressed(button + GLFW_KEY_LAST);
	}

	bool mouse_down(int button)
	{
		return key_down(button + GLFW_KEY_LAST);
	}

	bool mouse_up(int button)
	{
		return key_up(button + GLFW_KEY_LAST);
	}
};

extern InputUtil* input;

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	input->mouse_scroll = yoffset;
}

