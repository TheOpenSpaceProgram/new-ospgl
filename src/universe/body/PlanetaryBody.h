#pragma once
#include "../kepler/KeplerElements.h"
#include "config/PlanetConfig.h"
#include "../../renderer/PlanetaryBodyRenderer.h"
#include <glm/gtx/rotate_vector.hpp>

class PlanetaryBody
{
public:

	PlanetConfig config;

	PlanetaryBodyRenderer renderer;

	size_t index;

	std::string name;

	double soi_radius;

	// 0 = no dot, 1 = only dot
	// Used to save resources when rendering planets which are far away
	float dot_factor;

	// Set by PlanetarySystem deserializer
	PlanetaryBody* parent;
	ArbitraryKeplerOrbit orbit;

	// FOV is in radians
	float get_dot_factor(float distance, float fov);


	// In degrees per second
	double rotation_speed;
	// In degrees
	double rotation_at_epoch;
	// Relative to the ecliptic, aligned would mean (0, 1, 0)
	// It's constructed from the right ascension (towards vernal equinox, 'x')
	// and declination (towards north celestial pole, towards 'y')
	// in the config so the user does not need to touch this
	glm::dvec3 rotation_axis;

	PlanetaryBody();
	~PlanetaryBody();
};

template<>
class GenericSerializer<PlanetaryBody>
{
public:

	static glm::dvec3 get_rotation_axis(const cpptoml::table& from)
	{
		double np_declination, np_right_ascension;

		SAFE_TOML_GET(np_declination, "north_pole_declination", double);
		SAFE_TOML_GET(np_right_ascension, "north_pole_right_ascension", double);

		// Normalized earth position at t = 0, J2000
		// (Used to correct the coordinate system to a equinox aligned one)
		glm::dvec3 earth_at = -glm::dvec3(-0.18017889708209486, 0.00000000000000000, 0.98363385720819907);
		glm::dvec3 base = glm::dvec3(1.0, 0.0, 0.0);

		glm::dmat4 final_rot = MathUtil::rotate_from_to(earth_at, base);

		// Declination is given relative to the equator
		np_declination += 23.43671;


		glm::dmat4 decline = glm::rotate(glm::radians(np_declination), glm::dvec3(0.0, 0.0, -1.0));
		glm::dmat4 right_ascend = glm::rotate(glm::radians(np_right_ascension), glm::dvec3(0.0, 1.0, 0.0));

		return -glm::normalize(glm::dvec3(right_ascend * final_rot * decline * glm::dvec4(base, 1.0)));
	}

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

		static constexpr double REVS_PER_HOUR_TO_DEGREES_PER_SECOND = 0.1;
		//e
		SAFE_TOML_GET(to.rotation_at_epoch, "rotation_at_epoch", double);
		double rotation_period;
		SAFE_TOML_GET(rotation_period, "rotation_period", double);

		to.rotation_speed = (1.0 / rotation_period) * REVS_PER_HOUR_TO_DEGREES_PER_SECOND;

		to.rotation_axis = get_rotation_axis(from);
	}
};