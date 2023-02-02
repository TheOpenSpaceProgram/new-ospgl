#pragma once
#include "../LuaLib.h"

// Orbit prediction (both N-body and kepler) and
// drawing of the predictions, alongside propagation
// and trajectories
class LuaOrbit : public LuaLib
{
public:
	void load_to(sol::table& table) override;
};
