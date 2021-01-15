#pragma once
#include "AtmoConfig.h"
#include "SurfaceConfig.h"
#include "GasConfig.h"
#include <glm/glm.hpp>


struct ElementConfig
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
class GenericSerializer<ElementConfig>
{
public:

	NOT_SERIALIZABLE(ElementConfig);

	static void deserialize(ElementConfig& to, const cpptoml::table& from)
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