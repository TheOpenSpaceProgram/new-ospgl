#pragma once 
#include <glm/glm.hpp>

// Useful for blocking inputs to other parts of the engine while 
// the user is using GUIs, and for reducing boilerplate code
class GUIInput
{
private:

	bool mouse_int(int button = 0);

public:

	// Reset every frame, is X blocked FOR the GUI?
	bool ext_mouse_blocked = false;
	bool ext_scroll_blocked = false;
	bool ext_keyboard_blocked = false;

	// Reset every frame, is X used by the GUI?
	bool mouse_blocked = false;
	bool scroll_blocked = false;
	bool keyboard_blocked = false;

	// Blocks a mouse button from returning pressed until it's
	// released. Useful for buttons which change the GUI so additional
	// clicks aren't recorded
	bool block_mouse[3];

	// Call this before every frame, before scene update
	void update();

	bool mouse_inside(glm::ivec2 pos, glm::ivec2 size);
	bool mouse_inside(glm::ivec4 aabb);

	// To clear up the code a bit
	// 0 = left
	// 1 = right
	// 2 = middle
	// Usage of middle button is discouraged because, unlike the scrollbar,
	// some laptops users may not be able to use a scrollbar click
	#define GUI_LEFT_BUTTON 0
	#define GUI_RIGHT_BUTTON 1

	bool mouse_pressed(int button = 0);
	bool mouse_down(int button = 0);
	bool mouse_up(int button = 0);
	double mouse_scroll_delta() const;
};
