#pragma once 
#include <glm/glm.hpp>

// Useful for blocking inputs to other parts of the engine while 
// the user is using GUIs, and for reducing boilerplate code
class GUIInput
{
public:

	// Reset every frame, is moused blocked FOR the GUI?
	bool ext_mouse_blocked = false;
	// Reset every frame, is keyboard blocked FOR the GUI?
	bool ext_keyboard_blocked = false;

	// Reset every frame, is mouse used by the GUI?
	bool mouse_blocked = false;
	// Reset every frame, is keyboard used by the GUI?
	bool keyboard_blocked = false;

	// Call this before every frame, before scene update
	void update()
	{
		mouse_blocked = false;
		keyboard_blocked = false;
		ext_mouse_blocked = false;
		ext_keyboard_blocked = false;
	}

	bool is_mouse_inside(glm::ivec2 pos, glm::ivec2 size);
};
