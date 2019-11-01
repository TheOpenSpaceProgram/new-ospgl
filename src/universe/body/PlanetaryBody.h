#pragma once
#include "../kepler/KeplerElements.h"
#include "config/PlanetConfig.h"

class PlanetaryBody
{
public:

	PlanetConfig config;

	size_t index;

	std::string name;

	double soi_radius;

	// Set by PlanetarySystem deserializer
	PlanetaryBody* parent;
	ArbitraryKeplerOrbit orbit;




	PlanetaryBody();
	~PlanetaryBody();
};

template<>
class GenericSerializer<PlanetaryBody>
{
public:

	static void serialize(const PlanetaryBody& what, cpptoml::table& target)
	{

	}

	static void deserialize(PlanetaryBody& to, const cpptoml::table& from)
	{
		std::string config;
		SAFE_TOML_GET(config, "config", std::string);

		SAFE_TOML_GET(to.name, "name", std::string);

		::deserialize(to.orbit, from);

		auto config_toml = SerializeUtil::load_file(config);
		::deserialize(to.config, *config_toml);

	}
};