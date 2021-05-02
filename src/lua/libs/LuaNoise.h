#pragma once
#include "../LuaLib.h"

/*
	Implements FastNoise into lua, you must create a noise generator
	with ``new(seed)``, and call the functions there.

	Differences with FastNoise:

		- All enums are placed into the global table.
		- All names are changed to python_style
		- Some small renames:
			- cellular_distance_function -> cellular_distnace
			- cellular_return_type -> cellular_return
		- Cellular noise lookup is not implemented
		- get_cellular_distance2_indices returns the two indices
		- gradient_perturb is changed:
			- It returns 2 / 3 values (depending on number of arguments)
			- Or it returns a vec2 / vec3 (if called with vector types)

		- You can pass glm vectors to dimensional functions (get_x, etc...)
		- As usual, member access is done with ':'

	Note: You don't need to call ``[noise import name].noise.new(seed)`` (but that's possible), 
	 a shortcut (``[noise import name].new(seed)``) is created as the library is one class only.

 	TODO: Allow calling as fn = noise.new(seed) and then fn:[wathever](x, y)

*/
class LuaNoise : public LuaLib
{
public:

	void load_to(sol::table& table) override;
};

