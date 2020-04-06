#pragma once
#include <vector>
#include <cpptoml.h>
#include "../util/SerializeUtil.h"
#include "element/SystemElement.h"
#include "propagator/SystemPropagator.h"

#include <renderer/Drawable.h>

#pragma warning(push, 0)
#include <btBulletDynamicsCommon.h>
#pragma warning(pop)

#include <util/Logger.h>

class Universe;

// A system holds a central star, origin of the coordinate system, and
// many planets orbiting it using keplerian orbits
class PlanetarySystem : public Drawable
{
private:


	PosVector physics_pos;

	void render_body(CartesianState state, SystemElement* body, glm::dvec3 camera_pos, double t,
		glm::dmat4 proj_view, float far_plane);

	void render_body_atmosphere(CartesianState state, SystemElement* body, glm::dvec3 camera_pos, double t,
		glm::dmat4 proj_view, float far_plane);

	void update_render_body_rocky(PlanetaryBody* body, glm::dvec3 body_pos, glm::dvec3 camera_pos, double t);

	void update_physics(double dt, bool bullet);
	void init_physics(btDynamicsWorld* world);

	std::vector<glm::dvec3> pts;

public:

	Universe* universe;

	std::unordered_map<std::string, size_t> name_to_index;
	
	// Safer than directly indexing the array
	size_t get_element_index_from_name(const std::string& name);

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

	SystemPropagator* propagator;
	
	// Computes state of the whole system, including offsets, 
	// at a given time
	// We avoid allocating so you have to give a vector that's appropiately
	// sized (same as bodies.size() + 1), 0 is always the star
	void compute_states(double t, std::vector<CartesianState>& out, double tol = 1.0e-8);

	// Same as before but with positions
	void compute_positions(double t, std::vector<glm::dvec3>& out, double tol = 1.0e-8);

	void compute_sois(double t);

	glm::dvec3 get_gravity_vector(glm::dvec3 point, StateVector* states);

	virtual void deferred_pass(CameraUniforms& cu) override;
	virtual bool needs_deferred_pass() override { return true; }
	virtual void forward_pass(CameraUniforms& cu) override;
	virtual bool needs_forward_pass() override { return true; }

	void update(double dt, btDynamicsWorld* world, bool bullet);

	void init(btDynamicsWorld* world);

	// Updates LOD and similar
	// FOV in radians
	void update_render(glm::dvec3 camera_pos, float fov, double t);

	// Does the heavy loading
	void load(const cpptoml::table& root);

	PlanetarySystem(Universe* universe);
	~PlanetarySystem();
};


#include "PlanetarySystemSerializer.h"
