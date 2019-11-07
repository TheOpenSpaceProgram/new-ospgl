#pragma once
#include "kepler/KeplerElements.h"
#include "body/PlanetaryBody.h"
class SystemElement
{
public:

	size_t index;

	std::string name;

	bool is_barycenter;

	PlanetaryBody* as_body;

	// Only needed on bodies which orbit barycenters
	// If a body is a primary it does not need any orbital 
	// parameters other than the smajor_axis (which is computed
	// on load time)
	bool is_primary;

	// Only appears on barycenters
	SystemElement* barycenter_primary;
	SystemElement* barycenter_secondary;

	// Only appears on barycenter primaries
	double barycenter_radius;

	double soi_radius;

	// Set by PlanetarySystem deserializer
	SystemElement* parent;
	ArbitraryKeplerOrbit orbit;

	// Warning: Be careful when orbiting around barycenters
	double get_mass(bool as_primary = false)
	{
		if (is_barycenter)
		{
			if (as_primary)
			{
				return barycenter_secondary->as_body->config.mass;
			}
			else
			{
				return barycenter_primary->as_body->config.mass;
			}
		}
		else
		{
			return as_body->config.mass;
		}
	}

	SystemElement();
	~SystemElement();
};

template<>
class GenericSerializer<SystemElement>
{
public:

	static void serialize(const SystemElement& what, cpptoml::table& target)
	{
	}

	static void deserialize(SystemElement& to, const cpptoml::table& from)
	{
		to.as_body = nullptr;

		SAFE_TOML_GET(to.name, "name", std::string);

		SAFE_TOML_GET_OR(to.is_barycenter, "is_barycenter", bool, false);
		SAFE_TOML_GET_OR(to.is_primary, "is_primary", bool, false);

		if (!to.is_primary)
		{
			::deserialize(to.orbit, from);
		}

		if (!to.is_barycenter)
		{
			to.as_body = new PlanetaryBody();
			::deserialize(*to.as_body, from);
		}
	}
};
