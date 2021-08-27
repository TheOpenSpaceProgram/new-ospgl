#include "LuaPlumbing.h"
#include <universe/vehicle/plumbing/MachinePlumbing.h>
#include "LuaAssets.h"

void LuaPlumbing::load_to(sol::table &table)
{
	table.new_usertype<MachinePlumbing>("machine_plumbing",
	"create_port", &MachinePlumbing::create_port
	);

	table.new_usertype<StoredFluids>("stored_fluids",
	 sol::constructors<StoredFluids>(),
 		"get_contents", &StoredFluids::get_contents,
 		"add_fluid", [](StoredFluids& self, const LuaAssetHandle<PhysicalMaterial>& mat, float liquid, float gas)
 		{
			self.add_fluid(mat.get_asset_handle(), liquid, gas);
 		},
 		"modify", &StoredFluids::modify,
 		"multiply", &StoredFluids::multiply
 	);

	table.new_usertype<StoredFluid>("stored_fluid",
		"liquid_mass", &StoredFluid::liquid_mass,
		"gas_mass", &StoredFluid::gas_mass,
		"get_total_mass", &StoredFluid::get_total_mass);


}
