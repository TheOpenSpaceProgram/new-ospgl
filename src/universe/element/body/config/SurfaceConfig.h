#pragma once
#include <string>
#include <vector>
#include <OSP.h>
#include <util/SerializeUtil.h>
#include <util/serializers/glm.h>
#include <assets/Image.h>
#include <assets/AssetManager.h>

constexpr bool LOAD_SURFACES_ON_START = false;

struct SurfaceConfig
{
	std::string script_path;
	std::string script_path_raw;
	int max_depth;
	double coef_a;
	double coef_b;
	double coef_c;
	int depth_for_unload;

	bool has_water;

	// A rough estimate of maximum heigth from sea-level
	// Doesn't need to be really exact, but make sure it's
	// higher than the actual maximum height, otherwise physics 
	// will break
	double max_height;

};

template<>
class GenericSerializer<SurfaceConfig>
{
public:

	static void serialize(const SurfaceConfig& what, cpptoml::table& target)
	{
		target.insert("script_path", what.script_path_raw);
		target.insert("has_water", what.has_water);


		auto noise = cpptoml::make_table();


		target.insert("noise", noise);

		auto lod = cpptoml::make_table();

		lod->insert("max_depth", what.max_depth);
		lod->insert("coef_a", what.coef_a);
		lod->insert("coef_b", what.coef_b);
		
		target.insert("lod", lod);

	}

	static void deserialize(SurfaceConfig& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.has_water, "has_water", bool);
		SAFE_TOML_GET(to.script_path_raw, "script_path", std::string);
		SAFE_TOML_GET(to.max_depth, "lod.max_depth", int);
		SAFE_TOML_GET(to.coef_a, "lod.coef_a", double);
		SAFE_TOML_GET(to.coef_b, "lod.coef_b", double);
		SAFE_TOML_GET(to.coef_c, "lod.coef_c", double);
		SAFE_TOML_GET(to.depth_for_unload, "lod.depth_for_unload", int)

		SAFE_TOML_GET(to.max_height, "max_height", double);

		to.script_path = osp->assets->resolve_path(to.script_path_raw);
	}
};
