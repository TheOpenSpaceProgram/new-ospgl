#pragma once 
#include "../LuaLib.h"

// Note:
// The API is slightly different from cpptoml
// in order to adapt better to lua (it has no templates). 
// See examples to learn it
class LuaToml : public LuaLib 
{
public:

	virtual void load_to(sol::table& table);

};
