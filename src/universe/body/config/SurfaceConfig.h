#pragma once
#include <string>
#include <vector>
#include "../../../util/SerializeUtil.h"
#include "../../../util/serializers/glm.h"
#include "../../../assets/Image.h"
#include "../../../assets/AssetManager.h"

struct SurfaceConfig
{
	std::string script_path;
	std::string script_path_raw;
	int max_depth;
	double coef_a;
	double coef_b;

	int seed;
	int interp;

	bool has_water;

	std::unordered_map<std::string, Image*> images;
	std::unordered_map<std::string, std::string> image_paths;
};

template<>
class GenericSerializer<SurfaceConfig>
{
public:

	static void serialize(const SurfaceConfig& what, cpptoml::table& target)
	{
		target.insert("script_path", what.script_path_raw);
		target.insert("seed", what.seed);
		target.insert("has_water", what.has_water);


		auto noise = cpptoml::make_table();

		noise->insert("seed", what.seed);
		noise->insert("interp", what.interp);

		target.insert("noise", noise);

		auto lod = cpptoml::make_table();

		lod->insert("max_depth", what.max_depth);
		lod->insert("coef_a", what.coef_a);
		lod->insert("coef_b", what.coef_b);

		target.insert("lod", lod);

		auto images = cpptoml::make_table_array();
		for (auto it = what.image_paths.begin(); it != what.image_paths.end(); it++)
		{
			auto table = cpptoml::make_table();

			table->insert("name", it->first);
			table->insert("path", it->second);

			images->push_back(table);
		}

		if (what.image_paths.size() != 0)
		{
			target.insert("images", images);
		}

	}

	static void deserialize(SurfaceConfig& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.has_water, "has_water", bool);
		SAFE_TOML_GET(to.script_path_raw, "script_path", std::string);
		SAFE_TOML_GET(to.seed, "noise.seed", int);
		SAFE_TOML_GET(to.interp, "noise.interp", int);
		SAFE_TOML_GET(to.max_depth, "lod.max_depth", int);
		SAFE_TOML_GET(to.coef_a, "lod.coef_a", double);
		SAFE_TOML_GET(to.coef_b, "lod.coef_b", double);

		to.script_path = assets->resolve_path(to.script_path_raw);

		auto image_paths = from.get_table_array_qualified("images");
		if (image_paths)
		{
			for (const auto& table : *image_paths)
			{
				to.image_paths[table->get_qualified_as<std::string>("name").value_or("null")] = table->get_qualified_as<std::string>("path").value_or("ERROR");
			}

			// Actually load images
			for (auto it = to.image_paths.begin(); it != to.image_paths.end(); it++)
			{
				to.images[it->first] = assets->get_from_path<Image>(it->second);
			}
		}
	}
};