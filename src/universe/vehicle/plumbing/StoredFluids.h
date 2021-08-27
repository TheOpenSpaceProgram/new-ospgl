#pragma once
#include <unordered_map>
#include <assets/AssetManager.h>
#include <assets/PhysicalMaterial.h>


struct StoredFluid
{
	float liquid_mass;
	float gas_mass;


	float get_total_mass();
	StoredFluid();
	StoredFluid(float liquid, float gas);
};

// This may not be particularly fast, if it proves too slow, rewrite
// (It may be slow due to the use of a asset handle as key, probably there's
// a better way to do this)
class StoredFluids
{
private:

	// This vector allows fast access without allowing modifications to the material
	// regenerated after adding or removing fluids to improve perfomance
	std::vector<std::pair<const PhysicalMaterial*, StoredFluid*>> internal_contents;

	void generate_internal_contents();
	void remove_empty_fluids();

public:

	std::unordered_map<AssetHandle<PhysicalMaterial>, StoredFluid> contents;

	// Fluids in b may be negative, in that case it will take away
	// as much as possible and return the ammount taken
	StoredFluids modify(const StoredFluids& b);
	StoredFluids multiply(float value);

	// TODO: This may leak memory? (In lua, not sure)
	std::vector<std::pair<const PhysicalMaterial*, StoredFluid*>>* get_contents()
	{
		return &internal_contents;
	};

	// Modifies but without needing to manually create the StoredFluids, for lua
	void add_fluid(const AssetHandle<PhysicalMaterial>& mat, float liquid_mass, float gas_mass);


};

