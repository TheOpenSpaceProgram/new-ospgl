#pragma once
#include <unordered_set>
#include <tuple>
#include <assets/PhysicalMaterial.h>
#include <assets/AssetManager.h>

struct StoredFluid
{
	float gas_mass;
	float liquid_mass;
};

using TankContents = std::unordered_map<AssetHandle<PhysicalMaterial>, StoredFluid>;
using PartialPressures = std::unordered_map<AssetHandle<PhysicalMaterial>, float>;

// Fluid tanks are fixed volume!
// We refer to ullage as volume filled with gas, solids are not taken into
// account as of now (although they shouldn't happen on a fluid tank!)
class FluidTank
{
public:

	// m^3
	float volume;
	// m^3, space which can have gas but not liquid, useful to allow
	// fully filled tanks, so never set it to 0
	float extra_volume = 0.0001f;

	// K, average of all fluids to simulate heat diffusion between contents
	float temperature;

	// Value from 0 to 1, 0 meaning all ullage
	// is concentrated near the gas vent and
	// 1 meaning ullage is uniformly distributed through the tank
	// TODO: We could use 3D ullage in the future?
	// This current system doesn't simulate tanks being "reversed"
	float ullage_distribution;

	// Used for simulating column pressure of liquid under acceleration
	float last_acceleration;

	// Ullage formula: dU = -ax^3 + b, being x the acceleration
	float ullage_a_factor = 0.5f;
	float ullage_b_factor = 1e-5f;

	TankContents contents;

	// Pressure of the ullage portion (Pa)
	float get_pressure() const;
	float get_ullage_volume() const;
	float get_fluid_volume() const;

	// Finds the point where VP = P via Newton's method
	// It just does a few iterations at reducing dt until it stabilizes)
	void go_to_equilibrium(float max_dP = 0.1f);

	// Positive values means it's heated up, negative it's cooled
	void exchange_heat(float d);

	// Requests liquids from the tank, depending on ullage you may also
	// get fed some gases.
	// WE DONT SIMULATE FLUID DYNAMICS HERE! speed is simply the mass flow (kg/s)
	// if there was a 1atm pressure difference and no acceleration.
	// If ullage distribution is good, you will receive more dense contents first
	// BACKFLOW cant occur
	TankContents request_liquid(float speed, float dt, const PartialPressures& outside, bool only_liquid = false);
	// Requests gases from the tank, depending on ullage you may also
	// get fed some liquids
	// DOESN'T SIMULATE FLUID DYNAMICS EITHER, speed is mass flow (kg/s)
	// for a 1atm difference in pressure
	TankContents request_gas(float speed, float dt, const PartialPressures& outside, bool only_gas = false);

	// Updates fluid dynamics of the tank, including ullage distribution
	// Can be safely called on another thread or similar
	// Acceleration must be favourable for the tank (dot product of the vector and
	// the tank's "direction"), as it will reduce ullage distribution
	// If acceleration is negative, ullage will increase! Use that to simulate shaking and similar
	void update(float dt, float acceleration);

	void draw_imgui_debug();
};

