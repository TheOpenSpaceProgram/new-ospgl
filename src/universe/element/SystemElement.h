#pragma once
#include "../kepler/KeplerElements.h"
#include "config/ElementConfig.h"
#include <renderer/PlanetaryBodyRenderer.h>
#include <assets/Config.h>

class GroundShape;
class btRigidBody;

class SystemElement
{
public:
	size_t index;

	std::string name;

	bool nbody;

	ElementConfig config;

	PlanetaryBodyRenderer renderer;

	// Externally managed, these are not present on gas giants
	GroundShape* ground_shape;
	btRigidBody* rigid_body;


	// 0 = no dot, 1 = only dot
	// Used to save resources when rendering planets which are far away
	float dot_factor;


	// FOV is in radians
	float get_dot_factor(float distance, float fov);

	glm::dvec3 position_at_epoch;
	glm::dvec3 velocity_at_epoch;

	// In degrees per second
	double rotation_speed;
	// In degrees
	double rotation_at_epoch;
	// Relative to the ecliptic, aligned would mean (0, 1, 0)
	// It's constructed from the right ascension (towards vernal equinox, 'x')
	// and declination (towards north celestial pole, towards 'y')
	// in the config so the user does not need to touch this
	glm::dvec3 rotation_axis;

	glm::dmat4 build_rotation_matrix(double t0, double t, bool include_rot_at_epoch = true) const;

	double get_rotation_angle(double t0, double t, bool include_rot_at_epoch = true) const;
	double get_small_rotation_angle(double t0, double t, bool include_rot_at_epoch = true) const;

	// Coordimates are given relative to the rotated body
	// (Real rotation axis)
	glm::dvec3 get_tangential_speed(glm::dvec3 at_relative) const;

	double get_mass() const
	{
		return config.mass;
	}

	SystemElement();
	~SystemElement();
};

template<>
class GenericSerializer<SystemElement>
{
public:

	NOT_SERIALIZABLE(SystemElement);

	static void deserialize(SystemElement& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET_OR(to.name, "name", std::string, "");
		SAFE_TOML_GET_OR(to.nbody, "nbody", bool, false);

		std::string config;
		SAFE_TOML_GET(config, "config", std::string);

		AssetHandle<Config> cfg = AssetHandle<Config>(config);
		cfg->read_to(to.config);

		static constexpr double REVS_PER_HOUR_TO_DEGREES_PER_SECOND = 0.1;

		SAFE_TOML_GET(to.rotation_at_epoch, "rotation_at_epoch", double);
		double rotation_period;
		SAFE_TOML_GET(rotation_period, "rotation_period", double);

		to.rotation_speed = (1.0 / rotation_period) * REVS_PER_HOUR_TO_DEGREES_PER_SECOND;
		//to.rotation_speed = 0.0;

		SAFE_TOML_GET_TABLE(to.rotation_axis, "rotation_axis", glm::dvec3);
		to.rotation_axis = glm::normalize(to.rotation_axis);

		SAFE_TOML_GET_TABLE(to.position_at_epoch, "position", glm::dvec3);
		SAFE_TOML_GET_TABLE(to.velocity_at_epoch, "velocity", glm::dvec3);

	}
};
