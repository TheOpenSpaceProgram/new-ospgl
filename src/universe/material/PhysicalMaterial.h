#pragma once

// Temperatures in kelvin, pressures in atmospheres
// This is a simple simulation and ignores critical fluids and similar,
// it can handle solids, liquids and gases, although solids are simplified
struct PhysicalMaterial
{
	float std_bp;

	// J kg-1
	float dH_vaporization;
	float dH_fusion;

	// J kg-1
	float slh_evaporation;

	// J kg-1 K-1
	// For the isochoric process if possible
	float heat_capacity_solid;
	float heat_capacity_liquid;
	float heat_capacity_gas;

	// g / mol for convenience
	float molar_mass;

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