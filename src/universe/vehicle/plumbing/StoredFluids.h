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

	// TODO: There should me a special asset type for stuff like this. Fow now this works
	std::unordered_set<AssetHandle<PhysicalMaterial>> mat_ptrs;
	std::unordered_map<const PhysicalMaterial*, StoredFluid> contents;

	// Fluids in b may be negative, in that case it will take away
	// as much as possible and return the ammount taken
	StoredFluids modify(const StoredFluids& b);
	StoredFluids multiply(float value);

	// Note, iterate over pairs in lua like this:
	// for _, physical_material, stored_fluid = contents:pairs() do end
	// This is not noted anywhere on sol docs, so its useful to leave here
	const std::unordered_map<const PhysicalMaterial*, StoredFluid>& get_contents()
	{
		return contents;
	};

	// Introduces a new fluid (most used functionality) / modifies previously present fluid
	void add_fluid(const AssetHandle<PhysicalMaterial>& mat, float liquid_mass, float gas_mass);

	// Note that the quantities here represent draining, no negative needed
	void drain_to(StoredFluids* target, PhysicalMaterial* mat, float liquid_mass, float gas_mass, bool do_flow);

	// Assumes the fluids are uniformly mixed but dont dissolve, etc...
	float get_average_liquid_density() const;
	float get_total_liquid_mass() const;
	float get_total_liquid_volume() const;
	float get_total_gas_mass() const;
};

