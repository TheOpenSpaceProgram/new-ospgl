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

	NOT_SERIALIZABLE(AtmoConfig);

	static void deserialize(AtmoConfig& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.exponent, "exponent", double);
		SAFE_TOML_GET(to.sunset_exponent, "sunset_exponent", double);
		SAFE_TOML_GET(to.radius, "radius", double);
		SAFE_TOML_GET(to.kMie, "mie", double);
		SAFE_TOML_GET_TABLE(to.kRlh, "rayleigh", glm::vec3);
	}
};