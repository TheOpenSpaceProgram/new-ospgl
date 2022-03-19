#include "PhysicalMaterial.h"
#include <cmath>

constexpr float R = 8.314462618f;

float PhysicalMaterial::get_vapor_pressure(float T) const
{
	// We solve claysus clapeyron for pressure
	// We have 1 atm at the standard boiling point of the substance, so 101325Pa
	float lnP2 = -(dH_vaporization * molar_mass / R) * (1.0f / T - 1.0f / std_bp) + logf(101325.0f);
	return expf(lnP2);
}

float PhysicalMaterial::get_boiling_point(float P) const
{
	// We impose a reasonable limit on vacuum to avoid extrange results,
	// specially every boiling point going to 0K at pressure = 0atm
	P = fmaxf(P, 1e-20f);
	float H = dH_vaporization * molar_mass;
	// We solve claysus clapeyron for temperature instead
	// TODO: This may be badly solved, check it later
	return fmaxf(-std_bp * H / (std_bp * R * logf(P) - H) + logf(101325.0f), 0.0f);
}

float PhysicalMaterial::get_heat_capacity_increase(float T)
{
	return 1.0f + glm::pow(T, -5.0f);
}

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
		SAFE_TOML_GET(to.heat_capacity_gas, "heat_capacity_gas", double);
		SAFE_TOML_GET(to.heat_capacity_liquid, "heat_capacity_liquid", double);
		SAFE_TOML_GET(to.heat_capacity_solid, "heat_capacity_solid", double);
		SAFE_TOML_GET(to.dH_formation, "dH_formation", double);
		SAFE_TOML_GET(to.dH_vaporization, "dH_vaporization", double);
	}
};

PhysicalMaterial* load_physical_material(ASSET_INFO, const cpptoml::table &cfg)
{
	PhysicalMaterial* pmat = new PhysicalMaterial(ASSET_INFO_P);
	SerializeUtil::read_file_to(path, *pmat);
	return pmat;
}
