#pragma once
#include "InputContext.h"

// We implement a system inspired by this: 
// 	https://www.gamedev.net/blogs/entry/2250186-designing-a-robust-input-handling-system-for-games/
//
// The input system is divided into InputContexts which define:
// 	- Axes: Analogic input (but can be controlled by digital inputs) from -1 to 1
// 	- Actions: Digital input (you can detect when the button was pressed, released...)
// InputContexts are defined in lua, every controllable thing should define one.
// For easy user configuration mods should define a configuration page where their keybindings
// and axe-mappings can be changed for wathever InputContexts they implement.
//
class Input
{

};
