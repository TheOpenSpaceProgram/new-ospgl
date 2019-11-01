#pragma once
#include "AtmoConfig.h"
#include "SurfaceConfig.h"
#include "GasConfig.h"
#include <glm/glm.hpp>

struct PlanetConfig
{
	double mass;
	double radius;

	glm::vec3 far_color;

	bool has_atmo;
	AtmoConfig atmo;

	// If has_surface = false then the planet is a gas giant
	// (so it must either have a surface or a gas config)
	bool has_surface;
	SurfaceConfig surface;
	GasConfig gas;
};


template<>
class GenericSerializer<PlanetConfig>
{
public:

	static void serialize(const PlanetConfig& what, cpptoml::table& target)
	{
		if (what.has_atmo)
		{
			auto atmo = cpptoml::make_table();
			::serialize(what.atmo, *atmo);
			target.insert("atmo", atmo);
		}

		if (what.has_surface) 
		{
			auto surface = cpptoml::make_table();
			::serialize(what.surface, *surface);
			target.insert("surface", surface);
		}
		else
		{
			auto gas = cpptoml::make_table();
			::serialize(what.gas, *gas);
			target.insert("gas", gas);
		}

		target.insert("radius", what.radius);
		target.insert("mass", what.mass);
	
		serialize_to_table(what.far_color, target, "far_color");
	}

	static void deserialize(PlanetConfig& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.radius, "radius", double);
		SAFE_TOML_GET(to.mass, "mass", double);
		SAFE_TOML_GET_TABLE(to.far_color, "far_color", glm::vec3);

		if (from.get_table_qualified("atmo"))
		{
			to.has_atmo = true;
			::deserialize(to.atmo, *from.get_table_qualified("atmo"));
		}

		if (from.get_table_qualified("surface"))
		{
			to.has_surface = true;
			::deserialize(to.surface, *from.get_table_qualified("surface"));
		}
		else
		{
			to.has_surface = false;
			::deserialize(to.gas, *from.get_table_qualified("gas"));
		}

	}
};