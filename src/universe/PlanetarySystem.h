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

// The planetary system holds all the SystemElements and draws and updates them
class PlanetarySystem : public Drawable
{
private:


	PosVector physics_pos;

	static void render_body(CartesianState state, SystemElement* body, glm::dvec3 camera_pos, double t, double t0,
		glm::dmat4 proj_view, float far_plane);

	static void render_body_atmosphere(CartesianState state, SystemElement* body, glm::dvec3 camera_pos,
		glm::dmat4 proj_view, float far_plane);

	static void update_render_body_rocky(SystemElement* body, glm::dvec3 body_pos, glm::dvec3 camera_pos, double t, double t0);

	void update_physics(double dt, bool bullet);
	void init_physics(btDynamicsWorld* world);

	std::vector<glm::dvec3> pts;

public:

	double bt, t, t0;

	Universe* universe;

	std::unordered_map<std::string, size_t> name_to_index;
	std::vector<SystemElement*> elements;
	// How many n-body interacting elements are there? (The others are simply attracted)
	size_t nbody_count;
	
	// Safer than directly indexing the array
	size_t get_element_index_from_name(const std::string& name);

	StateVector states_now;
	// Updates with bullet physics dt instead of normal dt
	// TODO: Maybe the visual and physics states could
	// simply be this interpolated to save some CPU cycles
	// as planets don't really change direction much in the 
	// span of at most a few milliseconds
	StateVector bullet_states;

	SystemPropagator* propagator;
	
	glm::dvec3 get_gravity_vector(glm::dvec3 point, StateVector* states);

	void deferred_pass(CameraUniforms& cu, bool is_env_map) override;
	void forward_pass(CameraUniforms& cu, bool is_env_map) override;
	bool needs_deferred_pass() override { return true; }
	bool needs_forward_pass() override { return true; }
	bool needs_env_map_pass() override { return true; }

	void update(double dt, btDynamicsWorld* world, bool bullet);

	void init(btDynamicsWorld* world);

	// Updates LOD and similar, fov in radians
	void update_render(glm::dvec3 camera_pos, float fov);

	// Does the heavy loading of [[element]] objects
	void load(const cpptoml::table& root);

	explicit PlanetarySystem(Universe* universe);
	~PlanetarySystem();
};


