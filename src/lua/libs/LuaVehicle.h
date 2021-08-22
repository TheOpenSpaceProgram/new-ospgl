#include "../LuaLib.h"

// Loads classes to handle vehicles, pieces, parts and machines. It's unnecesary to store the return table
// as all functions will be exposed through other interfaces. For example, you may create pieces inside of a vehicle,
// but not by themselves. (Thus, using an already created vehicle)
class LuaVehicle : public LuaLib
{
public:
	virtual void load_to(sol::table& table) override;
};
