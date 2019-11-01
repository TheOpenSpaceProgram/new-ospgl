#pragma once
#include "../../../util/SerializeUtil.h"
#include "../../../util/serializers/glm.h"


struct AtmoConfig
{
	double exponent;
	double sunset_exponent;
	double radius;
	glm::vec3 main_color;
	glm::vec3 sunset_color;
};

template<>
class GenericSerializer<AtmoConfig>
{
public:

	static void serialize(const AtmoConfig& what, cpptoml::table& target)
	{
		target.insert("exponent", what.exponent);
		target.insert("sunset_exponent", what.sunset_exponent);
		target.insert("radius", what.radius);
		serialize_to_table(what.main_color, target, "main_color");
		serialize_to_table(what.sunset_color, target, "sunset_color");


	}

	static void deserialize(AtmoConfig& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.exponent, "exponent", double);
		SAFE_TOML_GET(to.sunset_exponent, "sunset_exponent", double);
		SAFE_TOML_GET(to.radius, "radius", double);
		SAFE_TOML_GET_TABLE(to.main_color, "main_color", glm::vec3);
		SAFE_TOML_GET_TABLE(to.sunset_color, "sunset_color", glm::vec3);
	}
};