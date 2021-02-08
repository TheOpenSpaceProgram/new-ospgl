#pragma once
#include <cpptoml.h>
#include <util/SerializeUtil.h>

struct RendererQuality
{



	int sun_shadow_size;
	int sun_terrain_shadow_size;
	int secondary_shadow_size;
	int env_map_size;

	struct PBR
	{
		enum class Quality
		{
			FULL, // Renders the scene to the envmap
			SIMPLE_ENVMAP, // Approximates the scene to the envmap
			SIMPLE // (Doesn't use envmapping)
		};

		// PBR samples are taken in this fashion:
		// p = 1.0 - planet_distance_parameter * planet_angular_size (radians);
		// sample   if distance_moved >= distance_per_sample * p
		// 			or time_per_sample >= time_per_sample * p
		// 			and frames_passed > frames_per_sample
		// if simple sampling is enabled,
		// sample 	if frames_passed > frames_per_sample
		Quality quality;
		int faces_per_sample;
		int frames_per_sample;
		double distance_per_sample;
		double time_per_sample;
		double planet_distance_parameter;
		bool simple_sampling;
	};

	PBR pbr;

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

		if(pbr.quality == PBR::Quality::FULL)
		{
			out += "#define _PBR_QUALITY_FULL\n";
		}
		else if(pbr.quality == PBR::Quality::SIMPLE_ENVMAP)
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

		std::string pbr_quality;
		SAFE_TOML_GET(pbr_quality, "pbr.quality", std::string);
		if(pbr_quality == "full")
			to.pbr.quality = RendererQuality::PBR::Quality::FULL;
		else if(pbr_quality == "simple_envmap")
			to.pbr.quality = RendererQuality::PBR::Quality::SIMPLE_ENVMAP;
		else if(pbr_quality == "simple")
			to.pbr.quality = RendererQuality::PBR::Quality::SIMPLE;
		else
			logger->fatal("Unknown pbr.quality setting: %s", pbr_quality);

		if(to.pbr.quality != RendererQuality::PBR::Quality::SIMPLE)
		{
			SAFE_TOML_GET_OR(to.pbr.simple_sampling, "pbr.simple_sampling", bool, false);

			if (!to.pbr.simple_sampling)
			{
				SAFE_TOML_GET(to.pbr.distance_per_sample, "pbr.distance_per_sample", double);
				SAFE_TOML_GET(to.pbr.planet_distance_parameter, "pbr.planet_distance_parameter", double);
				SAFE_TOML_GET(to.pbr.time_per_sample, "pbr.time_per_sample", double);
			}

			SAFE_TOML_GET(to.pbr.faces_per_sample, "pbr.faces_per_sample", int);
			SAFE_TOML_GET(to.pbr.frames_per_sample, "pbr.frames_per_sample", int);

		}

		SAFE_TOML_GET(to.atmosphere.iterations, "atmosphere.iterations", int);
		SAFE_TOML_GET(to.atmosphere.sub_iterations, "atmosphere.sub_iterations", int);
		SAFE_TOML_GET(to.atmosphere.low_end, "atmosphere.low_end", bool);
	}
};
