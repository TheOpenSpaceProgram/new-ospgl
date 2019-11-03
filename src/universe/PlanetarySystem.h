#pragma once
#include <vector>
#include <cpptoml.h>
#include "../util/SerializeUtil.h"
#include "body/PlanetaryBody.h"

#include "../renderer/camera/MapCamera.h"

// A system holds a central star, origin of the coordinate system, and
// many planets orbiting it using keplerian orbits
class PlanetarySystem
{
private:

	std::vector<CartesianState> render_states;

	void render_body(CartesianState state, PlanetaryBody* body, glm::dvec3 camera_pos, double t,
		glm::dmat4 proj_view, float far_plane);

	void update_render_body_rocky(PlanetaryBody* body, glm::dvec3 body_pos, glm::dvec3 camera_pos, double t);

public:

	// The MapCamera MAY not be here, actually (TODO)
	MapCamera camera;
	
	bool draw_debug;

	double star_mass;

	// Guaranteed to be ordered so that the last planets to appear
	// are moons, or moons of moons (etc...)
	std::vector<PlanetaryBody> bodies;
	
	// Computes state of the whole system, including offsets, 
	// at a given time
	// We avoid allocating so you have to give a vector that's appropiately
	// sized (same as bodies.size())
	void compute_states(double t, std::vector<CartesianState>& out, double tol = 1.0e-8);

	// Computes state of planets which can have a noticeable effect on our vehicle
	// It takes the index of the planet whose SOI we are inside, so moons of other
	// planets are ignored, and instead add to the central mass, so many of the return 
	// values in out are (0, 0) (with mass 0). These can be ignored!
	// SOI changes should be done as often as possible, but they are not required every
	// single step
	void compute_states_fast(double t, size_t soi_index, std::vector<CartesianState>& out, std::vector<glm::dvec3>& out_masses, double tol = 1.0e-8);

	// Computes SOIs for all bodies at a given time, as it only depends on the 
	// semi-major axis it's not really important which t you choose.
	void compute_sois(double t);

	void render(double t, int width, int height);
	
	void update(double dt);

	// Updates LOD and similar
	// FOV in radians
	void update_render(glm::dvec3 camera_pos, float fov, double t);


	PlanetarySystem();
	~PlanetarySystem();
};


#include "PlanetarySystemSerializer.h"