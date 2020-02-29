#pragma once
#include <vector>
#include <cpptoml.h>
#include "../util/SerializeUtil.h"
#include "vessel/Vessel.h"
#include "element/SystemElement.h"
#include "propagator/SystemPropagator.h"

#include "../renderer/camera/MapCamera.h"

#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)

struct SystemPointer;

// A system holds a central star, origin of the coordinate system, and
// many planets orbiting it using keplerian orbits
class PlanetarySystem
{
private:


	PosVector physics_pos;

	void render_body(CartesianState state, SystemElement* body, glm::dvec3 camera_pos, double t,
		glm::dmat4 proj_view, float far_plane);

	void update_render_body_rocky(PlanetaryBody* body, glm::dvec3 body_pos, glm::dvec3 camera_pos, double t);

	void update_physics(double dt, bool bullet);
	void init_physics(btDynamicsWorld* world);

	std::vector<glm::dvec3> pts;

public:

	std::unordered_map<std::string, size_t> name_to_index;
	
	StateVector states_now;
	// Updates with bullet physics dt instead of normal dt
	// TODO: Maybe the visual and physics states could
	// simply be this interpolated to save some CPU cycles
	// as planets don't really change direction much in the 
	// span of a few milliseconds
	StateVector bullet_states;

	double t, timewarp;
	double bt;

	// Guaranteed to be ordered so that the last planets to appear
	// are moons, or moons of moons (etc...)
	ElementVector elements;
	
	std::vector<Vessel> vessels;

	SystemPropagator* propagator;
	
	// Computes state of the whole system, including offsets, 
	// at a given time
	// We avoid allocating so you have to give a vector that's appropiately
	// sized (same as bodies.size() + 1), 0 is always the star
	void compute_states(double t, std::vector<CartesianState>& out, double tol = 1.0e-8);

	// Same as before but with positions
	void compute_positions(double t, std::vector<glm::dvec3>& out, double tol = 1.0e-8);

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

	void render(int width, int height, CameraUniforms& cu);
	void render_debug(int width, int height, CameraUniforms& cu);

	void update(double dt, btDynamicsWorld* world, bool bullet);

	void init(btDynamicsWorld* world);

	// Updates LOD and similar
	// FOV in radians
	void update_render(glm::dvec3 camera_pos, float fov, double t);

	PlanetarySystem();
	~PlanetarySystem();
};


#include "PlanetarySystemSerializer.h"