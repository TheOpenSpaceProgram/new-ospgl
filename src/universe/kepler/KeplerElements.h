#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../../util/SerializeUtil.h"
#include "../../util/Logger.h"



// Units are, always, meters (m), degrees (º), kilograms (kg)
// (Technically eccentricity is in radians)

// Easier to use and generate KeplerElements, they don't allow parameter
// variation and require central body mass. Used for generated in-game bodies
// where computing the variation is impossible, or for user-created bodies
// Mass of the central body is required to compute position
// It lacks any kind of anomaly as these are computed later on from time, 
// or from mean anomaly directly (if using NASA elements)

static constexpr double G = 6.67430e-11;


struct KeplerOrbit
{

	double smajor_axis;
	double eccentricity;
	double inclination;
	double periapsis_argument;
	double asc_node_longitude;

	// Unused by NASAKeplerOrbits as it gives mean anomaly directly
	double mean_at_epoch;

	double mean_to_eccentric(double mean, double tol = 1.0e-14) const;
	double eccentric_to_true(double eccentric) const;
	double time_to_mean(double time, double our_mass, double parent_mass) const;

	double get_period(double our_mass, double parent_mass) const;

	glm::dvec3 get_plane_normal();
};

struct CartesianState
{
	glm::dvec3 pos;
	glm::dvec3 vel;

	CartesianState() {}
	CartesianState(glm::dvec3 p, glm::dvec3 v) : pos(p), vel(v) {};
};

struct KeplerElements
{
	KeplerOrbit orbit;
	double true_anomaly;
	double eccentric_anomaly;
	double mean_anomaly;
	
	// Gets position relative to parent
	// X towards the equinox, y towards the solar system's up plane
	glm::dvec3 get_position();
	// Gets velocity vector relative to parent
	// Same coordinate system as get_position
	glm::dvec3 get_velocity(double parent_mass, double our_mass);

	// Faster than the previous two as data is shared between
	// position and velocity, use this one!
	CartesianState get_cartesian(double parent_mass, double our_mass);
};

// Converts a position and velocity state to orbital elements, given that
// the position and velocity is given relative to the wanted center body!
KeplerElements state_to_elements(glm::dvec3 rel_pos, glm::dvec3 rel_vel);

// Harder to generate elements, taken from NASA data for the default solar system.
// They don't require central body mass as it's included in the mean_longitude variation
// https://ssd.jpl.nasa.gov/txt/aprx_pos_planets.pdf
struct NASAKeplerOrbit
{
	// All values are given at epoch, and vary 
	// with the given per-century variation rates
	double smajor_axis;
	double smajor_axis_var;
	double eccentricity;
	double eccentricity_var;
	double inclination;
	double inclination_var;
	double mean_longitude;
	double mean_longitude_var;
	double periapsis_longitude;
	double periapsis_longitude_var;
	double asc_node_longitude;
	double asc_node_longitude_var;

	// Used on planets starting from Jupiter for extra precision
	bool has_extra;
	double b, c, s, f;

	// Time in seconds since J2000 epoch
	KeplerOrbit to_kepler_at(double t, double& mean_anomaly_out) const;

	KeplerElements to_elements_at(double time, double tol = 1.0e-14) const;
};


struct ArbitraryKeplerOrbit
{
	bool is_nasa_data;
	union Data
	{
		NASAKeplerOrbit nasa_data;
		KeplerOrbit normal_data;
	};

	Data data;

	KeplerElements to_elements_at(double time, double our_mass, double center_mass, double tol = 1.0e-14) const;
	KeplerOrbit to_orbit_at(double time);
};

template<>
class GenericSerializer<NASAKeplerOrbit>
{
public:
	static void serialize(const NASAKeplerOrbit& what, cpptoml::table& target)
	{
		target.insert("smajor_axis", what.smajor_axis);
		target.insert("smajor_axis_var", what.smajor_axis_var);

		target.insert("eccentricity", what.eccentricity);
		target.insert("eccentricity_var", what.eccentricity_var);

		target.insert("inclination", what.inclination);
		target.insert("inclination_var", what.inclination_var);

		target.insert("mean_longitude", what.mean_longitude);
		target.insert("mean_longitude_var", what.mean_longitude_var);

		target.insert("periapsis_longitude", what.periapsis_longitude);
		target.insert("periapsis_longitude_var", what.periapsis_longitude_var);

		target.insert("asc_node_longitude", what.asc_node_longitude);
		target.insert("asc_node_longitude_var", what.asc_node_longitude_var);

		if (what.has_extra)
		{
			target.insert("b", what.b);
			target.insert("c", what.c);
			target.insert("s", what.s);
			target.insert("f", what.f);
		}
	}

	static void deserialize(NASAKeplerOrbit& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.smajor_axis, "smajor_axis", double);
		SAFE_TOML_GET_OR(to.smajor_axis_var, "smajor_axis_var", double, 0.0);

		SAFE_TOML_GET(to.eccentricity, "eccentricity", double);
		SAFE_TOML_GET_OR(to.eccentricity_var, "eccentricity_var", double, 0.0);

		SAFE_TOML_GET(to.inclination, "inclination", double);
		SAFE_TOML_GET_OR(to.inclination_var, "inclination_var", double, 0.0);

		SAFE_TOML_GET(to.mean_longitude, "mean_longitude", double);
		SAFE_TOML_GET_OR(to.mean_longitude_var, "mean_longitude_var", double, 0.0);

		SAFE_TOML_GET(to.periapsis_longitude, "periapsis_longitude", double);
		SAFE_TOML_GET_OR(to.periapsis_longitude_var, "periapsis_longitude_var", double, 0.0);

		SAFE_TOML_GET(to.asc_node_longitude, "asc_node_longitude", double);
		SAFE_TOML_GET_OR(to.asc_node_longitude_var, "asc_node_longitude_var", double, 0.0);

		to.has_extra = false;

		// Extra parameters, if present
		if (from.get_qualified_as<double>("b"))
		{
			to.has_extra = true;
			SAFE_TOML_GET(to.b, "b", double);
			SAFE_TOML_GET(to.c, "c", double);
			SAFE_TOML_GET(to.s, "s", double);
			SAFE_TOML_GET(to.f, "f", double);
		}
	}
};


template<>
class GenericSerializer<KeplerOrbit>
{
public:
	static void serialize(const KeplerOrbit& what, cpptoml::table& target)
	{
		target.insert("smajor_axis", what.smajor_axis);
		target.insert("eccentricity", what.eccentricity);
		target.insert("inclination", what.inclination);
		target.insert("mean_at_epoch", what.mean_at_epoch);
		target.insert("periapsis_argument", what.periapsis_argument);
		target.insert("asc_node_longitude", what.asc_node_longitude);
	}

	static void deserialize(KeplerOrbit& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET(to.smajor_axis, "smajor_axis", double);
		SAFE_TOML_GET(to.eccentricity, "eccentricity", double);
		SAFE_TOML_GET(to.inclination, "inclination", double);
		SAFE_TOML_GET(to.mean_at_epoch, "mean_at_epoch", double);
		SAFE_TOML_GET(to.periapsis_argument, "periapsis_argument", double);
		SAFE_TOML_GET(to.asc_node_longitude, "asc_node_longitude", double);
	}
};


template<>
class GenericSerializer<ArbitraryKeplerOrbit>
{
public:
	static void serialize(const ArbitraryKeplerOrbit& what, cpptoml::table& target)
	{
		target.insert("is_nasa_data", what.is_nasa_data);

		if (what.is_nasa_data)
		{
			::serialize(what.data.nasa_data, target);
		}
		else
		{
			::serialize(what.data.normal_data, target);
		}
	}

	static void deserialize(ArbitraryKeplerOrbit& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET_OR(to.is_nasa_data, "is_nasa_data", bool, false);

		if (to.is_nasa_data)
		{
			::deserialize(to.data.nasa_data, from);
		}
		else
		{
			::deserialize(to.data.normal_data, from);
		}
	}


};
