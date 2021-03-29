#pragma once
#include "Asset.h"
#include <string>
#include <util/SerializeUtil.h>

// Temperatures in kelvin, pressures in atmospheres
// This is a simple simulation and ignores critical fluids and similar,
// it can handle solids, liquids and gases, although solids are simplified
struct PhysicalMaterial : public Asset
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
	// TODO: Give the option of using a polynomial approximation? Scientific data is given like that
	float heat_capacity_solid;
	float heat_capacity_liquid;
	float heat_capacity_gas;

	// kg / mol for convenience
	float molar_mass;

	// kg / m^3
	float liquid_density;
	float solid_density;

	// Pa * s
	float liquid_viscosity;
	float gas_viscosity;

	// Uses the clausius-clapeyron equation, approximate but good enough
	float get_vapor_pressure(float T) const;
	float get_boiling_point(float P) const;

	// Returns a number to be multiplied by heat capacity to avoid
	// 0K being reachable (unrealistic approach, good enough)
	static float get_heat_capacity_increase(float T);

	inline float get_moles(float mass) const
	{
		return mass / molar_mass;
	}

	inline float get_mass(float moles) const
	{
		return moles * molar_mass;
	}

	PhysicalMaterial(ASSET_INFO) : Asset(ASSET_INFO_P) {}

};

PhysicalMaterial* load_physical_material(ASSET_INFO, const cpptoml::table& cfg);

