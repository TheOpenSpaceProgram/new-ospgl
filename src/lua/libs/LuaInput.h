#pragma once
#include "../LuaLib.h"

// Wrapper around InputUtil. Not to be used by machines for their input, but directly
// by scenes to implement interaction. Remember to also use gui_input for proper blocking.
class LuaInput : public LuaLib
{
public:

	virtual void load_to(sol::table& table) override;

};
