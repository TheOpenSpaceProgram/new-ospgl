#pragma once
#include "InputContext.h"

// We implement a system inspired by this: 
// 	https://www.gamedev.net/blogs/entry/2250186-designing-a-robust-input-handling-system-for-games/
//
// The input system is divided into InputContexts which define:
// 	- Axes: Analogic input (but can be controlled by digital inputs) from -1 to 1
// 	- Actions: Digital input (you can detect when the button was pressed, released...)
// InputContexts are defined in a config, controllable machines have a function named
// get_input_context which returns the context to use so the user can select which machine
// to control
//
// This is only used in the Flight scene (it controls machines)
class FlightInput
{
private:

	InputContext* cur_ctx = nullptr;

public:

	bool keyboard_blocked = false;

	void set_ctx(InputContext* ctx);

	void update(GLFWwindow* window, double dt);

};
