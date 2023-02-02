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
// Emits events when any element is removed / created so anything which
// holds elements can update indices:
// core:system_update_indices() <- Called when indices need to be updated
// core:system_element_removed(old_id, elem_name) <- Called when an element is removed
// core:system_element_created(new_id, elem_name) <- Called when an element is created
class PlanetarySystem : public Drawable, public Propagable
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

	// We only interpolate position for bullet, as velocity is "implicit"
	// (and wouldn't even change a measurable quantity)
	glm::dvec3 interp_pos(size_t elem_id);
	void interp_pos(CartesianState& st);
public:

	// Used to prevent predictors from getting garbage data
	std::mutex lock;

	double bt, t, t0;

	Universe* universe;

	std::unordered_map<std::string, size_t> name_to_index{};
	std::vector<SystemElement*> elements{};
	// How many n-body interacting elements are there? (The others are simply attracted)
	size_t nbody_count;
	
	// Safer than directly indexing the array
	size_t get_element_index_from_name(const std::string& name);

	CartesianState get_element_state(const std::string& name, bool physics = false);
	CartesianState get_element_state(size_t elem_id, bool physics = false);
	// Some short-hands, slight optimization for lua
	glm::dvec3 get_element_position(const std::string& name);
	glm::dvec3 get_element_velocity(const std::string& name);
	SystemElement* get_element(const std::string& name);

	StateVector states_now;
	// Only used during time-warp, to propagate entities
	LightStateVector handled_states_now;
	TrajectoryVector handled_states_trj;

	SystemPropagator* propagator;
	
	glm::dvec3 get_gravity_vector(glm::dvec3 point, bool physics);

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

	StateVector* get_massful_states() override
	{
		return &states_now;
	};
	LightStateVector* get_light_states() override
	{
		return &handled_states_now;
	}
	TrajectoryVector* get_trajectories() override
	{
		return &handled_states_trj;
	}

	explicit PlanetarySystem(Universe* universe);
	~PlanetarySystem();
};


