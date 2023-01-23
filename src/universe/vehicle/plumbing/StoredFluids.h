#pragma once
#include <unordered_map>
#include <unordered_set>
#include <assets/AssetManager.h>
#include <assets/PhysicalMaterial.h>
#include <lua/libs/LuaAssets.h>


struct StoredFluid
{
	float liquid_mass;
	float gas_mass;


	float get_total_mass();
	StoredFluid();
	StoredFluid(float liquid, float gas);
};

class StoredFluids
{
private:

	// This vector allows fast access without allowing modifications to the material
	// regenerated after adding or removing fluids to improve perfomance
	std::vector<std::pair<const PhysicalMaterial*, StoredFluid*>> internal_contents;

	void remove_empty_fluids();

public:

	// Temperature of the fluids
	float temperature;
	// TODO: There should me a special asset type for stuff like this. Fow now this works
	std::unordered_set<AssetHandle<PhysicalMaterial>> mat_ptrs;
	std::unordered_map<const PhysicalMaterial*, StoredFluid> contents;
	std::unordered_map<std::string, const PhysicalMaterial*> name_to_ptr;

	// Fluids in b may be negative, in that case it will take away
	// as much as possible and return the ammount taken
	StoredFluids modify(const StoredFluids& b);
	StoredFluids multiply(float value);

	const std::unordered_map<const PhysicalMaterial*, StoredFluid>& get_contents()
	{
		return contents;
	};

	// Changes vapor / liquid distribution "aphysically", ie, it maintains temperature without
	// the need for energy to be added.
	// Do not use if effects of vaporization are expected to be significant!
	void set_vapor_fraction(const AssetHandle<PhysicalMaterial>& mat, float factor);

	// Introduces a new fluid (most used functionality) / modifies previously present fluid
	// Negative temperature means it's added at the temperature of the already existing fluids
	void add_fluid(const AssetHandle<PhysicalMaterial>& mat, float liquid_mass, float gas_mass, float temp = -1.0f);

	// Note that the quantities here represent draining, no negative needed
	void drain_to(StoredFluids* target, PhysicalMaterial* mat, float liquid_mass, float gas_mass, bool do_flow);

	// Carries out possible chemical reactions given the temperature
	// Returns heat released
	float react(float T, float react_V, float liquid_react_factor, float dt);

	// Assumes the fluids are uniformly mixed but dont dissolve, etc...
	float get_average_liquid_density() const;
	float get_total_liquid_mass() const;
	float get_total_liquid_volume() const;
	float get_total_gas_mass() const;
	float get_total_heat_capacity() const;
	float get_total_gas_moles() const;
	// Obtains R / Mm (Mm is the mean molar mass for the mixture)
	float get_specific_gas_constant() const;
	void add_heat(float heat);
};

