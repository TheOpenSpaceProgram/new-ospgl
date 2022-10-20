#include "../LuaLib.h"

// Loads classes to handle vehicles, pieces, parts and machines. It's unnecesary to store the return table
// as all functions will be exposed through other interfaces. For example, you may create pieces inside of a vehicle,
// but not by themselves. (Thus, using an already created vehicle)
//
// WorldStates are passed using a lua table with the following members:
//   - pos: vec3 (must have)
//   - vel: vec3 (must have)
//   - rot: quat (must have)
//   - angvel: vec3 (optional, defaults to (0,0,0))
// WARNING: When drawing vehicles, do not hold the returned Node* from get_model_node as it may
// disappear after long enough!
class LuaVehicle : public LuaLib
{
public:
	virtual void load_to(sol::table& table) override;
};
