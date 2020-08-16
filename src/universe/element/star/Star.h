#pragma once

struct Star
{
	double radius;
	double mass;
	
};

template<>
class GenericSerializer<Star>
{
public:

	static void serialize(const Star& what, cpptoml::table& target)
	{
	}

	static void deserialize(Star& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.mass, "mass", double);
		SAFE_TOML_GET(to.radius, "radius", double);
	}
};