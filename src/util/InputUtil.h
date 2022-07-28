#pragma once
#include <glm/glm.hpp>
#define GLFW_INCLUDE_NONE 
#include <GLFW/glfw3.h>
#include <cstring>
#include <string>
// For some reason Visual Studio compiler doesnt state correct C++ version, even if it's using >= C++11
#define UTF_CPP_CPLUSPLUS 201103L
#include <utf8/utf8.h>

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void char_callback(GLFWwindow* window, unsigned int codepoint);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);


// Central class for making GLFW inputs easy to handle
// TODO: An option to simulate middle click may be useful for the few users who dont have a mouse with scrollbar?
class InputUtil
{
private:

	GLFWcursor* std_cursors[10];

public:

	// May contain unicode stuff! be careful
	std::string input_text;

	enum class Cursor
	{
		NORMAL,
		IBEAM,
		CROSSHAIR,
		HAND,
		RESIZE_EW,
		RESIZE_NS,
		RESIZE_NWSE, //< ! Not supported on many platforms
		RESIZE_NESW, //< ! Not supported on many platforms
		RESIZE_ALL,
		NOT_ALLOWED					
	};

	glm::dvec2 prev_mouse_pos;
	glm::dvec2 mouse_pos;
	glm::dvec2 mouse_delta;

	double mouse_scroll_delta;
	double prev_mouse_scroll;
	double mouse_scroll;

	static const size_t KEY_LIST_SIZE = GLFW_KEY_LAST + GLFW_MOUSE_BUTTON_LAST;

	bool current_key_status[KEY_LIST_SIZE] = {false};
	bool prev_key_status[KEY_LIST_SIZE] = {false};

	// if positive, a key is repeating this frame
	int repeating_key;

	GLFWwindow* window;

	void setup(GLFWwindow* window)
	{
		glfwSetScrollCallback(window, scroll_callback);
		glfwSetCharCallback(window, char_callback);
		glfwSetKeyCallback(window, key_callback);
		this->window = window;
		std_cursors[(int)Cursor::NORMAL] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		std_cursors[(int)Cursor::IBEAM] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
		std_cursors[(int)Cursor::CROSSHAIR] = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
		std_cursors[(int)Cursor::HAND] = glfwCreateStandardCursor(GLFW_POINTING_HAND_CURSOR);
		std_cursors[(int)Cursor::RESIZE_EW] = glfwCreateStandardCursor(GLFW_RESIZE_EW_CURSOR);
		std_cursors[(int)Cursor::RESIZE_NS] = glfwCreateStandardCursor(GLFW_RESIZE_NS_CURSOR);
		std_cursors[(int)Cursor::RESIZE_NWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
		std_cursors[(int)Cursor::RESIZE_NESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
		std_cursors[(int)Cursor::RESIZE_ALL] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
		std_cursors[(int)Cursor::NOT_ALLOWED] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
	}

	void update(GLFWwindow* window)
	{
		// Remember we are called before glfwPolLEvents and all other game code!
		repeating_key = -1;
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

	bool key_down_or_repeating(int key)
	{
		return key_down(key) || repeating_key == key;
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

	void set_cursor(Cursor cursor)
	{
		glfwSetCursor(window, std_cursors[(int)cursor]);
	}
	// Clears the text input and returns read keys
	std::string get_input_text()
	{
		std::string out = input_text;
		input_text.clear();
		return out;
	}
};

extern InputUtil* input;

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	input->mouse_scroll = yoffset;
}

static void char_callback(GLFWwindow* window, unsigned int code_point)
{
	std::u32string in_str;
	in_str += code_point;
	input->input_text.append(utf8::utf32to8(in_str));
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_REPEAT)
	{
		input->repeating_key = key;
	}
}
