#pragma once
#include <unordered_set>
#include <tuple>
#include "PhysicalMaterial.h"

struct StoredFluid
{
	float gas_mass;
	float liquid_mass;
};

// Fluid tanks are fixed volume!
// We refer to ullage as volume filled with gas, solids are not taken into
// account as of now (although they shouldn't happen on a fluid tank!)
class FluidTank
{
public:

	// m^3
	float volume;
	// K
	float temperature;

	// Value from 0 to 1, 0 meaning all ullage
	// is concentrated near the gas vent (top of the tank) and
	// 1 meaning ullage is uniformly distributed through the tank
	// TODO: We could use 3D ullage in the future?
	// This current system doesn't simulate tanks being "reversed"
	float ullage_distribution;

	// Ullage formula: dU = -ax^3 + b, being x the acceleration
	float ullage_a_factor = 0.5f;
	float ullage_b_factor = 1e-5f;

	// The string is the ID of the material
	std::unordered_map<std::string, StoredFluid> contents;

	// Pressure of the ullage portion
	float get_pressure() const;
	float get_ullage_volume() const;
	float get_fluid_volume() const;

	// Lets mostly gases escape (unless ullage distribution is too bad)
	// Similar to a hole at the "top" of the tank
	// Gases will leave per partial pressure, but lower density liquids
	// may leave too
	// Speed in kg/s atm
	std::vector<StoredFluid> vent(float speed, float dt, bool only_gas = false);
	// Lets mostly liquids escape (unless ullage distribution is too bad)
	// Similar to a pump at the "bottom" of the tank, lower density
	// fluids will go first if ullage distribution is good, otherwise
	// a mix will occur
	// Speed in kg/s
	std::vector<StoredFluid> pump(float speed, float dt, bool only_liquid = false);

	// Updates fluid dynamics of the tank, including ullage distribution
	// Can be safely called on another thread or similar
	// Acceleration must be favourable for the tank (dot product of the vector and
	// the tank's "direction"), as it will reduce ullage distribution
	// If acceleration is negative, ullage will increase! Use that to simulate shaking and similar
	void update(float dt, float acceleration);

	void draw_imgui_debug();
};

