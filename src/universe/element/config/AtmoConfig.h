#pragma once
#include <util/SerializeUtil.h>
#include <util/serializers/glm.h>


struct AtmoConfig
{
	double exponent;
	double sunset_exponent;
	double radius;
	glm::vec3 kRlh;
	float kMie;
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
		target.insert("mie", what.kMie);
		serialize_to_table(what.kRlh, target, "rayleigh");


	}

	static void deserialize(AtmoConfig& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.exponent, "exponent", double);
		SAFE_TOML_GET(to.sunset_exponent, "sunset_exponent", double);
		SAFE_TOML_GET(to.radius, "radius", double);
		SAFE_TOML_GET(to.kMie, "mie", double);
		SAFE_TOML_GET_TABLE(to.kRlh, "rayleigh", glm::vec3);
	}
};