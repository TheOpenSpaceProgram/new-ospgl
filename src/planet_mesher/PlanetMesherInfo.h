#pragma once
#include <cpptoml.h>
#include "../util/SerializeUtil.h"
// Handles all long-term storage information of a planet
class PlanetMesherInfo
{
public:

	double radius;
	std::string script_path;

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
		target.insert("script_path", what.script_path);
	}

	static void deserialize(PlanetMesherInfo& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.radius, "radius", double);
		SAFE_TOML_GET(to.script_path, "script_path", std::string);
	}
};