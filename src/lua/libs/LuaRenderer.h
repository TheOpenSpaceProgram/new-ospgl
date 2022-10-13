#pragma once
#include "../LuaLib.h"

/*
 * If you want to add a drawable, you will need to have its type defined in lua so the casting
 * can work. Ie, if you want to do:
 * 		renderer:add_drawable(universe.system)
 * you will need to have:
 * 		require("universe") -- which contains planetary_system
 * or otherwise you will get a runtime error
 *
 * Also includes functions to create CameraUniforms (ie, for writing cameras!)
 * but it's a better idea to use the functions in core/scenes/cameras.lua
 */
class LuaRenderer : public LuaLib
{
public:
	void load_to(sol::table& table) override;
};



