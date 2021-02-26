#include "PhysicalMaterial.h"
#include <cmath>

constexpr float R = 8.314462618f;

float PhysicalMaterial::get_vapor_pressure(float T) const
{
	// We solve claysus clapeyron for pressure
	// We have 1 atm at the standard boiling point of the substance
	float lnP2 = -(slh_evaporation * molar_mass * 1e-3f / R) * (1.0f / T - 1.0f / std_bp);
	return expf(lnP2);
}

float PhysicalMaterial::get_boiling_point(float P) const
{
	// We impose a reasonable limit on vacuum to avoid extrange results,
	// specially every boiling point going to 0K at pressure = 0atm
	P = fmaxf(P, 1e-20f);
	float H = slh_evaporation * molar_mass * 1e-3f;
	// We solve claysus clapeyron for temperature instead
	return fmaxf(-std_bp * H / (std_bp * R * logf(P) - H), 0.0f);
}
