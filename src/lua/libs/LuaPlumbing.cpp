#include "LuaPlumbing.h"
#include <universe/vehicle/plumbing/MachinePlumbing.h>

void LuaPlumbing::load_to(sol::table &table)
{
	table.new_usertype<MachinePlumbing>("machine_plumbing",
		"create_port", &MachinePlumbing::create_port
	);

	table.new_usertype<StoredFluids>("stored_fluids",
		 sol::constructors<StoredFluids>(),
		 "get_contents", &StoredFluids::get_contents
	 );

}
