#include "../LuaLib.h"

class LuaVehicle : public LuaLib
{
public:
	virtual void load_to(sol::table& table) override;
};
