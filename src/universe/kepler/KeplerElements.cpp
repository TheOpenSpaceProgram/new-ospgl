#include "KeplerElements.h"

#define ORBIT_COS cos
#define ORBIT_SIN sin

KeplerOrbit NASAKeplerOrbit::to_kepler_at(double time, double& mean_anomaly_out) const
{
	// Centuries since J2000.0
	
	constexpr double DAYS_PER_CENTURY = 36525.0;
	constexpr double SECONDS_PER_DAY = 86400;
	double t = (time / SECONDS_PER_DAY) / DAYS_PER_CENTURY;

	// Compute orbital elements now

	double smajor_axis_now = smajor_axis + t * smajor_axis_var;
	double eccentricity_now = eccentricity + t * eccentricity_var;
	double inclination_now = inclination + t * inclination_var;
	double mean_longitude_now = mean_longitude + t * mean_longitude_var;
	double periapsis_longitude_now = periapsis_longitude + t * periapsis_longitude_var;
	double asc_node_longitude_now = asc_node_longitude + t * asc_node_longitude;

	// Compute argument of periapsis and mean anomaly, include extra parameters if needed
	double periapsis_argument_now = periapsis_longitude_now - asc_node_longitude_now;
	double mean_anomaly_now = mean_longitude_now - periapsis_longitude_now;
	if (has_extra)
	{
		mean_anomaly_now += b * t * t + c * ORBIT_COS(f * t) + s * ORBIT_SIN(f * t);
	}

	// Contain mean anomaly in [-180, +180]
	// TODO: Is this correct
	//mean_anomaly_now = fmod(mean_anomaly_now, 180.0);


	KeplerOrbit out;

	out.smajor_axis = smajor_axis_now;
	out.eccentricity = eccentricity_now;
	out.inclination = inclination_now;
	out.periapsis_argument = periapsis_argument_now;
	out.asc_node_longitude = asc_node_longitude_now;
	mean_anomaly_out = mean_anomaly_now;

	glm::cos(1.0);

	return out;
}

KeplerElements NASAKeplerOrbit::to_elements_at(double time, double tol) const
{
	KeplerElements elems;

	double mean_anom;
	KeplerOrbit orbit = to_kepler_at(time, mean_anom);

	double ecc_anom = orbit.mean_to_eccentric(mean_anom, tol);
	double true_anom = orbit.eccentric_to_true(ecc_anom);

	elems.orbit = orbit;
	elems.true_anomaly = true_anom;
	elems.eccentric_anomaly = ecc_anom;
	elems.mean_anomaly = mean_anom;

	return elems;
}


// Implementation of these procedures:
// http://alpheratz.net/dynamics/twobody/KeplerIterations_summary.pdf
static inline double starting_value(double ecc, double mean)
{
	double t34 = ecc * ecc;
	double t35 = ecc * t34;
	double t33 = ORBIT_COS(mean);


	return mean + ((-1.0 / 2.0) * t35 + ecc + (t34 + (3.0 / 2.0) * t33 * t35) * t33) * ORBIT_SIN(mean);
}

static inline double eps3(double ecc, double mean, double x)
{
	double t1 = ORBIT_COS(x);
	double t2 = -1 + ecc * t1;
	double t3 = ORBIT_SIN(x);
	double t4 = ecc * t3;
	double t5 = -x + t4 + mean;
	double t6 = t5 / ((1.0 / 2.0) * t5 * (t4 / t2) + t2);
	return t5 / (((1.0 / 2.0) * t3 - (1.0 / 6.0) * t1 * t6) * ecc * t6 + t2);
}

static inline double elliptic_iterative(double mean, double eccentricity, double tol)
{
	double out = 0.0;

	double mnorm = fmod(mean, 2.0 * glm::pi<double>());
	double e0 = starting_value(eccentricity, mnorm);
	double de = tol + 1;
	double count = 0;
	while (de > tol)
	{
		out = e0 - eps3(eccentricity, mnorm, e0);
		de = abs(out - e0);
		e0 = out;
		count++;
		if (count >= 100)
		{
			logger->fatal("Kepler iterative solver failed, this should never happen! Check tolerance");
			return 0; // FATAL: Many iterations
		}
	}

	return out;
}


double KeplerOrbit::mean_to_eccentric(double mean, double tol) const
{
	if (eccentricity < 1.0)
	{
		// Elliptic solver
		return elliptic_iterative(glm::radians(mean), eccentricity, tol);
	}
	else if (eccentricity >= 1.0)
	{
		// Hyperbolic / Parabolic solver (TODO)
		logger->fatal("Tried to solve a non-elliptic orbit, which is not yet implemented");
		return 0.0;
	}

	return 0.0;
}

double KeplerOrbit::eccentric_to_true(double eccentric) const
{
	double half = glm::radians(eccentric) / 2.0;
	if (eccentricity < 1.0)
	{
		return 2.0 * atan2(sqrt(1 + eccentricity) * ORBIT_SIN(half), sqrt(1 - eccentricity) * cos(half));
	}
	else
	{
		// Hyperbolic / Parabolic solver (TODO)
		logger->fatal("Tried to solve a non-elliptic orbit, which is not yet implemented");
		return 2.0 * atan(sqrt((eccentricity + 1.0) / (eccentricity - 1.0)) * tanh(eccentric / 2.0));
	}
}

double KeplerOrbit::time_to_mean(double time, double our_mass, double parent_mass) const
{
	double sm = std::abs(smajor_axis);
	if (sm == 0)
	{
		return 0;
	}


	double n = sqrt((G * (our_mass + parent_mass)) / (sm * sm * sm));
	return -n * time + mean_at_epoch;
}

double KeplerOrbit::get_period(double our_mass, double parent_mass) const
{
	if (eccentricity >= 1.0)
	{
		return std::numeric_limits<double>::infinity();
	}

	double numerator = smajor_axis * smajor_axis * smajor_axis;
	double denominator = G * (our_mass + parent_mass);

	return 2.0 * glm::pi<double>() * sqrt(numerator / denominator);
}

#include <glm/gtx/normal.hpp>

glm::dvec3 KeplerOrbit::get_plane_normal()
{
	// TODO: Make this method better, this is a disaster
	// and it's probably slow too
	KeplerElements a = KeplerElements();  a.orbit = *this;
	a.eccentric_anomaly = 0.0;
	KeplerElements b = KeplerElements();  b.orbit = *this;
	b.eccentric_anomaly = 1.0;
	KeplerElements c = KeplerElements();  c.orbit = *this;
	c.eccentric_anomaly = 2.0;

	return -glm::triangleNormal(a.get_position(), b.get_position(), c.get_position());
}

KeplerElements ArbitraryKeplerOrbit::to_elements_at(double time, double our_mass, double center_mass, double tol) const
{
	if (is_nasa_data)
	{
		return data.nasa_data.to_elements_at(time, tol);
	}
	else
	{
		KeplerElements elems;

		elems.orbit = data.normal_data;

		double mean = data.normal_data.time_to_mean(time, our_mass, center_mass);
		double ecc = data.normal_data.mean_to_eccentric(mean, tol);
		elems.true_anomaly = data.normal_data.eccentric_to_true(ecc);
		elems.eccentric_anomaly = ecc;
		elems.mean_anomaly = mean;

		return elems;
	}
}

KeplerOrbit ArbitraryKeplerOrbit::to_orbit_at(double time)
{
	if (is_nasa_data)
	{
		double mean;
		return data.nasa_data.to_kepler_at(time, mean);
	}
	else
	{
		return data.normal_data;
	}
}

glm::dvec3 KeplerElements::get_position()
{
	glm::dvec3 out;

	glm::dvec2 flat;
	flat.x = orbit.smajor_axis * (ORBIT_COS(eccentric_anomaly) - orbit.eccentricity);
	flat.y = orbit.smajor_axis * sqrt(1 - orbit.eccentricity * orbit.eccentricity) * ORBIT_SIN(eccentric_anomaly);

	double w = glm::radians(orbit.periapsis_argument);
	double O = glm::radians(orbit.asc_node_longitude);
	double I = glm::radians(orbit.inclination);


	// Note: The coordinates in the JPL file change y for z
	double xx = ORBIT_COS(w) * ORBIT_COS(O) - ORBIT_SIN(w) * ORBIT_SIN(O) * ORBIT_COS(I);
	double xy = -ORBIT_SIN(w) * ORBIT_COS(O) - ORBIT_COS(w) * ORBIT_SIN(O) * ORBIT_COS(I);
	

	double yx = ORBIT_SIN(w) * ORBIT_SIN(I);
	double yy = ORBIT_COS(w) * ORBIT_SIN(I);

	double zx = ORBIT_COS(w) * ORBIT_SIN(O) + ORBIT_SIN(w) * ORBIT_COS(O) * ORBIT_COS(I);
	double zy = -ORBIT_SIN(w) * ORBIT_SIN(O) + ORBIT_COS(w) * ORBIT_COS(O) * ORBIT_COS(I);

	out.x = xx * flat.x + xy * flat.y;
	out.y = yx * flat.x + yy * flat.y;
	out.z = zx * flat.x + zy * flat.y;

	return out;
}

CartesianState KeplerElements::get_cartesian(double parent_mass, double our_mass)
{
	glm::dvec3 pos;
	glm::dvec3 vel;


	glm::dvec2 flat;
	double sinE = ORBIT_SIN(eccentric_anomaly);
	double cosE = ORBIT_COS(eccentric_anomaly);
	double E2 = orbit.eccentricity * orbit.eccentricity;
	double sqrt1mE2 = sqrt(1 - E2);
	flat.x = orbit.smajor_axis * (cosE - orbit.eccentricity);
	flat.y = orbit.smajor_axis * sqrt1mE2 * sinE;

	double w = glm::radians(orbit.periapsis_argument);
	double O = glm::radians(orbit.asc_node_longitude);
	double I = glm::radians(orbit.inclination);


	// Note: The coordinates in the JPL file change y for z
	double xx = ORBIT_COS(w) * ORBIT_COS(O) - ORBIT_SIN(w) * ORBIT_SIN(O) * ORBIT_COS(I);
	double xy = -ORBIT_SIN(w) * ORBIT_COS(O) - ORBIT_COS(w) * ORBIT_SIN(O) * ORBIT_COS(I);


	double yx = ORBIT_SIN(w) * ORBIT_SIN(I);
	double yy = ORBIT_COS(w) * ORBIT_SIN(I);

	double zx = ORBIT_COS(w) * ORBIT_SIN(O) + ORBIT_SIN(w) * ORBIT_COS(O) * ORBIT_COS(I);
	double zy = -ORBIT_SIN(w) * ORBIT_SIN(O) + ORBIT_COS(w) * ORBIT_COS(O) * ORBIT_COS(I);

	pos.x = xx * flat.x + xy * flat.y;
	pos.y = yx * flat.x + yy * flat.y;
	pos.z = zx * flat.x + zy * flat.y;

	double dist2 = flat.y * flat.y + flat.x + flat.x;
	double p = parent_mass * our_mass * G;

	double mult = sqrt((p * orbit.smajor_axis) / dist2);

	glm::dvec2 flat_vel;
	flat_vel.x = mult * -sinE;
	flat_vel.y = mult * sqrt1mE2 * cosE;

	// Transform the same as pos
	vel.x = xx * flat_vel.x + xy * flat_vel.y;
	vel.y = yx * flat_vel.x + yy * flat_vel.y;
	vel.z = zx * flat_vel.x + zy * flat_vel.y;

	return CartesianState(pos, vel);
}
