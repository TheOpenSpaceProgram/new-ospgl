#include "LuaPlumbing.h"
#include <universe/vehicle/plumbing/PlumbingMachine.h>
#include "LuaAssets.h"

void LuaPlumbing::load_to(sol::table &table)
{
	table.new_usertype<PlumbingMachine>("machine_plumbing",
										"create_port", &PlumbingMachine::create_port
	);

	table.new_usertype<StoredFluids>("stored_fluids",
	 sol::constructors<StoredFluids()>(),
		"get_contents", &StoredFluids::get_contents,
		"add_fluid", [](StoredFluids& self, const LuaAssetHandle<PhysicalMaterial>& mat, float liquid, float gas)
		{
			self.add_fluid(mat.get_asset_handle(), liquid, gas);
		},
		 // This one is meant to be used in loops that modify a tank
		"modify_fluid", [](StoredFluids& self, const PhysicalMaterial* mat, float liquid, float gas)
		{
			self.add_fluid(AssetHandle<PhysicalMaterial>(mat->get_asset_id()), liquid, gas);
		},
		 "set_vapor_fraction", [](StoredFluids& self, const PhysicalMaterial* mat, float factor)
		 {
			 self.set_vapor_fraction(AssetHandle<PhysicalMaterial>(mat->get_asset_id()), factor);
		 },
 		"modify", &StoredFluids::modify,
 		"multiply", &StoredFluids::multiply,
	 	"drain_to", &StoredFluids::drain_to,
		 "temperature", &StoredFluids::temperature,
		 "get_total_liquid_volume", &StoredFluids::get_total_liquid_volume,
		 "get_total_liquid_mass", &StoredFluids::get_total_liquid_mass,
		 "get_total_gas_mass", &StoredFluids::get_total_gas_mass,
		 "add_heat", &StoredFluids::add_heat,
		 "get_total_gas_moles", &StoredFluids::get_total_gas_moles,
		 "get_average_liquid_density", &StoredFluids::get_average_liquid_density,
		 "react", &StoredFluids::react,
		sol::meta_function::to_string, [](const StoredFluids& f)
		{
			std::string list = "Fluid contents: ";
			for(const auto& pair : f.contents)
			{
				std::string substr = fmt::format("{}: {{{} kg liquid, {} kg gas}}; ", pair.first->name,
												 pair.second.liquid_mass, pair.second.gas_mass);
				list += substr;
			}
			return list;
		}
 	);

	table.new_usertype<StoredFluid>("stored_fluid",
		"liquid_mass", &StoredFluid::liquid_mass,
		"gas_mass", &StoredFluid::gas_mass,
		"get_total_mass", &StoredFluid::get_total_mass);

	// TODO: Move this to another file
	table.new_usertype<PhysicalMaterial>("physical_material",
		 "name", &PhysicalMaterial::name,
		 "formula", &PhysicalMaterial::formula,
		 "std_bp", &PhysicalMaterial::std_bp,
		 "dH_vaporization", &PhysicalMaterial::dH_vaporization,
		 "dH_fusion", &PhysicalMaterial::dH_fusion,
		 "dH_formation", &PhysicalMaterial::dH_formation,
		 "heat_capacity_solid", &PhysicalMaterial::heat_capacity_solid,
		 "heat_capacity_liquid", &PhysicalMaterial::heat_capacity_liquid,
		 "heat_capacity_gas", &PhysicalMaterial::heat_capacity_gas,
		 "molar_mass", &PhysicalMaterial::molar_mass,
		 "liquid_density", &PhysicalMaterial::liquid_density,
		 "solid_density", &PhysicalMaterial::solid_density,
		 "get_vapor_pressure", &PhysicalMaterial::get_vapor_pressure,
		 "get_boiling_point", &PhysicalMaterial::get_boiling_point,
		 "get_heat_capacity_increase", &PhysicalMaterial::get_heat_capacity_increase,
		 "get_moles", &PhysicalMaterial::get_moles,
		 "get_mass", &PhysicalMaterial::get_mass);

}
