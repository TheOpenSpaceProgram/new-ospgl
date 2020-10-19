#pragma once
#include <cpptoml.h>
#include <util/SerializeUtil.h>

struct RendererQuality
{
	int sun_shadow_size;
	int sun_terrain_shadow_size;
	int secondary_shadow_size;
	int env_map_size;

	
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
	}
};
