#pragma once
#include <string>
#include <util/SerializeUtil.h>

// Temperatures in kelvin, pressures in atmospheres
// This is a simple simulation and ignores critical fluids and similar,
// it can handle solids, liquids and gases, although solids are simplified
struct PhysicalMaterial
{
	std::string name;
	std::string formula;

	float std_bp;

	// J kg-1
	float dH_vaporization;
	float dH_fusion;
	float dH_formation;

	// J kg-1 K-1
	// For the isochoric process if possible
	float heat_capacity_solid;
	float heat_capacity_liquid;
	float heat_capacity_gas;

	// g / mol for convenience
	float molar_mass;

	float liquid_density;
	float solid_density;

	// Uses the clausius-clapeyron equation, approximate but good enough
	float get_vapor_pressure(float T) const;
	float get_boiling_point(float P) const;

	inline float get_moles(float mass) const
	{
		return (mass * 1000.0f) / molar_mass;
	}

	inline float get_mass(float moles) const
	{
		return moles * molar_mass * 1e-3f;
	}

};

template<>
class GenericSerializer<PhysicalMaterial>
{
public:

	static void deserialize(PhysicalMaterial& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.name, "name", std::string);
		SAFE_TOML_GET_OR(to.formula, "formula", std::string, "N/A");
		SAFE_TOML_GET(to.std_bp, "std_bp", double);
		SAFE_TOML_GET(to.molar_mass, "molar_mass", double);
		SAFE_TOML_GET(to.liquid_density, "liquid_density", double);
		SAFE_TOML_GET(to.solid_density, "solid_density", double);
		SAFE_TOML_GET(to.dH_formation, "dH_formation", double);
		SAFE_TOML_GET(to.dH_vaporization, "dH_vaporization", double);
	}
};

