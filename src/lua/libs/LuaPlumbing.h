#include "../LuaLib.h"

// Allows the usage of these types:
// - StoredFluids
// - PlumbingMachine
// And returns a table containing the following classes that you may create:
// - StoredFluids
class LuaPlumbing : public LuaLib
{
public:
	void load_to(sol::table& table) override;
};

