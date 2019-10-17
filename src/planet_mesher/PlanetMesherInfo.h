#pragma once
#include <cpptoml.h>
#include "../util/SerializeUtil.h"
// Handles all long-term storage information of a planet
class PlanetMesherInfo
{
public:

	double radius;
	double atmo_radius;
	std::string script_path;
	int max_depth;
	double coef_a;
	double coef_b;

	int seed;
	int interp;

	bool has_water;

	PlanetMesherInfo();
	~PlanetMesherInfo();
};


template<>
class GenericSerializer<PlanetMesherInfo>
{
public:

	static void serialize(const PlanetMesherInfo& what, cpptoml::table& target)
	{
		target.insert("radius", what.radius);
		target.insert("atmo_radius", what.radius);
		target.insert("script_path", what.script_path);
		target.insert("seed", what.seed);

		auto noise = cpptoml::make_table();

		noise->insert("seed", what.seed);
		noise->insert("interp", what.interp);

		target.insert("noise", noise);

		auto lod = cpptoml::make_table();

		lod->insert("max_depth", what.max_depth);
		lod->insert("coef_a", what.coef_a);
		lod->insert("coef_b", what.coef_b);

		target.insert("lod", lod);

		target.insert("has_water", what.has_water);
	}

	static void deserialize(PlanetMesherInfo& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.has_water, "has_water", bool);
		SAFE_TOML_GET(to.radius, "radius", double);
		SAFE_TOML_GET(to.atmo_radius, "atmo_radius", double);
		SAFE_TOML_GET(to.script_path, "script_path", std::string);
		SAFE_TOML_GET(to.seed, "noise.seed", int);
		SAFE_TOML_GET(to.interp, "noise.interp", int);
		SAFE_TOML_GET(to.max_depth, "lod.max_depth", int);
		SAFE_TOML_GET(to.coef_a, "lod.coef_a", double);
		SAFE_TOML_GET(to.coef_b, "lod.coef_b", double);

	}
};