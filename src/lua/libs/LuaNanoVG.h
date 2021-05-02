#pragma once
#include "../LuaLib.h"

// We use FFI, so NanoVG cannot be used as an object and must instead
// be used in the same way it's used in C++, passing vg to all calls
class LuaNanoVG : public LuaLib
{
	void load_to(sol::table& table) override;

};
