#pragma once
#include "../kepler/KeplerElements.h"
#include "body/PlanetaryBody.h"
#include "barycenter/Barycenter.h"
#include "star/Star.h"

class SystemElement
{
public:

	enum SystemElementType
	{
		STAR,
		BARYCENTER,
		BODY
	};

	size_t index;

	std::string name;

	SystemElementType type;

	PlanetaryBody* as_body;
	Barycenter* as_barycenter;
	Star* as_star;

	// Only needed on bodies which orbit barycenters
	// If a body is a primary it does not need any orbital 
	// parameters other than the smajor_axis (which is computed
	// on load time)
	bool is_primary;


	double soi_radius;

	// Set by PlanetarySystem deserializer
	SystemElement* parent;
	ArbitraryKeplerOrbit orbit;
	// Applied to barycenter primaries only
	double barycenter_radius;

	// Warning: Be careful when orbiting around barycenters
	double get_mass(bool as_primary = false, bool compound = false)
	{
		if (type == STAR)
		{
			return as_star->mass;
		}
		else if (type == BARYCENTER)
		{
			if (compound)
			{
				return as_barycenter->primary->as_body->config.mass + as_barycenter->secondary->as_body->config.mass;
			}
			else
			{
				if (as_primary)
				{
					return as_barycenter->secondary->as_body->config.mass;
				}
				else
				{
					return as_barycenter->primary->as_body->config.mass;
				}
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

	// Deserialize is only called for bodies and barycenters
	// Star is special
	static void deserialize(SystemElement& to, const cpptoml::table& from)
	{
		to.as_body = nullptr;

		SAFE_TOML_GET(to.name, "name", std::string);

		bool is_barycenter;

		SAFE_TOML_GET_OR(is_barycenter, "is_barycenter", bool, false);
		SAFE_TOML_GET_OR(to.is_primary, "is_primary", bool, false);

		if (!to.is_primary)
		{
			::deserialize(to.orbit, from);
		}

		if (is_barycenter)
		{
			to.type = SystemElement::BARYCENTER;
			to.as_barycenter = new Barycenter();
		}
		else
		{
			to.type = SystemElement::BODY;
			to.as_body = new PlanetaryBody();
			::deserialize(*to.as_body, from);

		}
	}
};
