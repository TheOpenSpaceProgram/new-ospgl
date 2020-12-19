#pragma once
#include <cpptoml.h>
#include <util/SerializeUtil.h>

struct RendererQuality
{
	enum class PBRQuality
	{
		FULL,
		SIMPLE_ENVMAP,
		SIMPLE // (Doesn't use envmapping)
	};

	int sun_shadow_size;
	int sun_terrain_shadow_size;
	int secondary_shadow_size;
	int env_map_size;

	PBRQuality pbr_quality;

	struct Atmosphere
	{
		bool low_end;
		int iterations;
		int sub_iterations;
	};

	Atmosphere atmosphere;


	std::string get_shader_defines() const
	{
		std::string out;

		out += "#ifndef _QUALITY_DEFINES_INCLUDED\n";

		if(pbr_quality == PBRQuality::FULL)
		{
			out += "#define _PBR_QUALITY_FULL\n";
		}
		else if(pbr_quality == PBRQuality::SIMPLE_ENVMAP)
		{
			out += "#define _PBR_QUALITY_SIMPLE_ENVMAP\n";
		}
		else
		{
			out += "#define _PBR_QUALITY_SIMPLE\n";
		}

		out += "#define _ATMO_ITERATIONS " + std::to_string(atmosphere.iterations); out += "\n";
		out += "#define _ATMO_SUB_ITERATIONS " + std::to_string(atmosphere.sub_iterations); out += "\n";
		if(atmosphere.low_end)
		{
			out += "#define _ATMO_LOW_END\n";
		}


		out += "#define _QUALITY_DEFINES_INCLUDED\n";
		out += "#endif\n";

		return out;
	}
	
};


template<>
class GenericSerializer<RendererQuality>
{
public:

	static void serialize(const RendererQuality& what, cpptoml::table& target)
	{
		logger->fatal("Not implemented");
	}

	static void deserialize(RendererQuality& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.sun_shadow_size, "sun_shadow_size", int);
		SAFE_TOML_GET(to.sun_terrain_shadow_size, "sun_terrain_shadow_size", int);
		SAFE_TOML_GET(to.secondary_shadow_size, "secondary_shadow_size", int);
		SAFE_TOML_GET(to.env_map_size, "env_map_size", int);

		SAFE_TOML_GET(to.atmosphere.iterations, "atmosphere.iterations", int);
		SAFE_TOML_GET(to.atmosphere.sub_iterations, "atmosphere.sub_iterations", int);
		SAFE_TOML_GET(to.atmosphere.low_end, "atmosphere.low_end", bool);
	}
};
