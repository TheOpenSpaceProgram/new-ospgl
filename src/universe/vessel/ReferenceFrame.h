#pragma once

#include <glm/gtx/quaternion.hpp>
#include "../SystemPointer.h"

class PlanetarySystem;



struct ReferenceFrame
{
	enum ReferenceMode
	{
		// Origin is the center of the center body
		// Y axis goes towards the North Pole
		// X axis goes towards the equator
		// On vessels and barycenters this returns global axes
		INERTIAL,
		// Origin is the center of the body
		// X-Axis goes towards the planet's equatorial axis
		// Y-Axis goes towards the planet's north pole
		// On barycenters this aligns the two bodies
		// It rotates with the vessel's forward. Useful for the camera
		ROTATING,
		// Origin is at a point of the surface of the body
		// X-Axis points to the north (compass style)
		// Y-Axis goes to space through that point from the body's center
		// Useful for landings as the horizon is actually the horizon and
		// the surface point is fixed. Mostly used by the navball
		SURFACE,

		// Origin is at the center body
		// X-Axis goes towards that body's parent
		// Y-Axis goes towards the body's orbit's plane normal
		// Useful for both rendezvous and for keeping, say, your panels
		// aligned towards the sun
		// Be careful as it won't ignore barycenters! If you want
		// the Earth-Sun aligned system, choose the Earth-Moon barycenter
		// as center, otherwise it will display the Earth-Earth.Moon system
		PARENT_ALIGNED,

		// Origin is at the center of 'center'
		// X-Axis goes towards 'center1'
		// Y-Axis goes towards the cross product of the X-axis
		// and the velocity of the second body relative to the first
		// relative to the first one, "fixing" the orbital plane
		ALIGNED,
		
		// Origin is at the barycenter of the two bodies
		// X-Axis goes from the barycenter to 'center1'
		// Y-Axis goes towards the cross product of the bodies
		// velocities
		ALIGNED_BARYCENTRIC,

	};

	SystemPointer center;
	std::optional<SystemPointer> center2;
	ReferenceMode mode;
	
	// Normalized coordinates, only used on Surface mode
	// coordinates are non-rotated coordinates, so (0, 1, 0) is the 
	// north pole
	glm::dvec3 surface_point;



	glm::dvec3 get_x_axis(double t = std::numeric_limits<double>::infinity()) const;
	glm::dvec3 get_y_axis(double t = std::numeric_limits<double>::infinity()) const;
	glm::dvec3 get_z_axis(double t = std::numeric_limits<double>::infinity()) const;

	glm::dmat4 get_rotation_matrix() const;

	glm::dvec3 get_velocity() const;
	glm::dvec3 get_center() const;

	void draw_debug_axes() const;

	ReferenceFrame(SystemPointer c) : center(c)
	{
	}
};