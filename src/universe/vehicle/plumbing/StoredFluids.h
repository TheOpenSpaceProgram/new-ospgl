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
};

// This may not be particularly fast, if it proves too slow, rewrite
// (It may be slow due to the use of a asset handle as key, probably there's
// a better way to do this)
class StoredFluids
{
private:

public:

	std::unordered_map<AssetHandle<PhysicalMaterial>, StoredFluid> contents;

	// Fluids in b may be negative, in that case it will take away
	// as much as possible and return the ammount taken
	StoredFluids modify(const StoredFluids& b);
	StoredFluids multiply(float value);
	// Readonly physical material, but you may modify the StoredFluid
	std::vector<std::pair<const PhysicalMaterial*, StoredFluid*>> get_contents();
};

