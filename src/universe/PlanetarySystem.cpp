#include "PlanetarySystem.h"
#include "../util/DebugDrawer.h"
#include <imgui/imgui.h>
#include "../physics/glm/BulletGlmCompat.h"
#include "../physics/ground/GroundShape.h"
#include <game/GameState.h>

glm::dvec3 PlanetarySystem::get_gravity_vector(glm::dvec3 p, bool physics)
{
	glm::dvec3 result = glm::dvec3(0, 0, 0);
	for(size_t i = 0; i < states_now.size(); i++)
	{
		double mass = elements[i]->get_mass();

		CartesianState st = states_now[i];
		if(physics)
		{
			interp_pos(st);
		}
		glm::dvec3 diff = st.pos - p;

		double dist2 = glm::length2(diff);

		glm::dvec3 diffn = diff / glm::sqrt(dist2);
		double am = (G * mass) / dist2;
		result += diffn * am;
	}

	return result;
}

void PlanetarySystem::render_body(CartesianState state, SystemElement* body, glm::dvec3 camera_pos, double t, double t0,
	glm::dmat4 proj_view, float far_plane)
{

	if(body->render_enabled == false)
		return;

	glm::dvec3 camera_pos_relative = camera_pos - state.pos;
	glm::dvec3 light_dir = glm::normalize(state.pos);

	glm::dmat4 model = glm::translate(glm::dmat4(1.0), -camera_pos + state.pos);
	model = glm::scale(model, glm::dvec3(body->config.radius));

	glm::dmat4 rot_matrix = body->build_rotation_matrix(t0, t);

	glm::dmat4 dmodel = glm::translate(glm::dmat4(1.0), -camera_pos + state.pos);
	// We scale down to small coordinates
	//dmodel = glm::scale(dmodel, glm::dvec3(1.0 / body->config.radius));

	PlanetRenderer::PlanetRenderTforms tforms;
	tforms.wmodel = model * rot_matrix;
	tforms.normal_matrix = rot_matrix;
	tforms.far_plane = far_plane;
	tforms.camera_pos = camera_pos_relative;
	tforms.time = t;
	tforms.light_dir = light_dir;
	tforms.rot = body->get_small_rotation_angle(t0, t);
	tforms.rot_tform = glm::transpose(glm::inverse(rot_matrix));
	tforms.proj_view = proj_view;

	body->renderer.deferred(tforms, body->config, body->dot_factor);


	if (debug_drawer->debug_enabled)
	{
		body->renderer.draw_debug(t0, t, state, body);
	}
}

void PlanetarySystem::render_body_atmosphere(CartesianState state, SystemElement * body, glm::dvec3 camera_pos,
											 glm::dmat4 proj_view, float far_plane)
{
	glm::dvec3 camera_pos_relative = camera_pos - state.pos;
	glm::dvec3 light_dir = glm::normalize(state.pos);

	body->renderer.forward(proj_view, camera_pos_relative, body->config, far_plane, light_dir);
}


#include "../util/InputUtil.h"

static double zoom = 1.0;


void PlanetarySystem::deferred_pass(CameraUniforms& cu, bool is_env_map)
{
	// TODO: Don't have this fixed
	float fov = glm::radians(90.0f);

	glm::dvec3 camera_pos = cu.cam_pos;

	if(!is_env_map)
	{
		update_render(camera_pos, fov);
	}

	glm::dmat4 proj_view = cu.proj_view;
	glm::dmat4 c_model = cu.c_model;
	float far_plane = cu.far_plane;

	for (size_t i = 0; i < elements.size(); i++)
	{
		render_body(states_now[i], elements[i], camera_pos, t, t0, proj_view, far_plane);
	}

	if (debug_drawer->debug_enabled)
	{
		// 1 AU
		double axis_length = 149597900000;
		debug_drawer->add_point(glm::dvec3(0, 0, 0), glm::vec3(1.0, 1.0, 0.5));

		debug_drawer->add_line(glm::dvec3(0, 0, 0), glm::dvec3(1, 0, 0) * axis_length, glm::vec3(1.0, 0.0, 0.0));
		debug_drawer->add_line(glm::dvec3(0, 0, 0), glm::dvec3(0, 1, 0) * axis_length, glm::vec3(0.0, 1.0, 0.0));
		debug_drawer->add_line(glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 1) * axis_length, glm::vec3(0.0, 0.0, 1.0));

	}
}

void PlanetarySystem::forward_pass(CameraUniforms& cu, bool is_env_map)
{
	glm::dvec3 camera_pos = cu.cam_pos;
	glm::dmat4 proj_view = cu.proj_view;
	glm::dmat4 c_model = cu.c_model;
	float far_plane = cu.far_plane;

	using BodyPositionPair = std::pair<SystemElement*, CartesianState>;

	// Sort bodies by distance to camera to avoid weird atmospheres
	std::vector<BodyPositionPair> sorted;

	for (size_t i = 0; i < elements.size(); i++)
	{
		sorted.emplace_back(elements[i], states_now[i]);
	}

	std::sort(sorted.begin(), sorted.end(), [camera_pos](BodyPositionPair a, BodyPositionPair b)
	{
		return glm::distance2(camera_pos, a.second.pos) > glm::distance2(camera_pos, b.second.pos);
	});

	for (size_t i = 0; i < sorted.size(); i++)
	{
		render_body_atmosphere(sorted[i].second, sorted[i].first, camera_pos, proj_view, far_plane);
	}
}


void PlanetarySystem::update_physics(double dt, bool bullet)
{
	if(!bullet)
	{
		lock.lock();
		propagator->propagate(dt);
		lock.unlock();
		t += dt;
	}

	if (bullet)
	{
		bt += dt;
		// Give data to colliders
		for(size_t i = 0; i < elements.size(); i++)
		{
			SystemElement* elem = elements[i];
			if(elem->config.has_surface)
			{
				btTransform tform = btTransform::getIdentity();
				tform.setOrigin(to_btVector3(interp_pos(i)));
				glm::dmat4 mat = elem->build_rotation_matrix(t0, bt);
				glm::dquat quat = glm::dquat(mat);

				tform.setRotation(to_btQuaternion(quat));

				elem->rigid_body->setWorldTransform(tform);
			}
		}
	}
	else
	{
		t += dt;
	}

}

void PlanetarySystem::init_physics(btDynamicsWorld* world)
{
	// Create the colliders and rigidbodies

	for(auto elem : elements)
	{
		if(elem->config.has_surface)
		{
			elem->ground_shape = new GroundShape(elem);
			elem->rigid_body = new btRigidBody(1000000000.0, nullptr, elem->ground_shape, btVector3(0, 0, 0));
			elem->rigid_body->setCollisionFlags(
					elem->rigid_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			elem->rigid_body->setFriction(1.0);

			elem->rigid_body->setRestitution(1.0);
			elem->ground_shape->setMargin(2.0);
			elem->rigid_body->setActivationState(DISABLE_DEACTIVATION);

			world->addRigidBody(elem->rigid_body);
		}
	}


}

size_t PlanetarySystem::get_element_index_from_name(const std::string& name)
{
	auto it = name_to_index.find(name);
	// TODO: Maybe make this not important?
	logger->check(it != name_to_index.end(), "Tried to access by name ({}) a element which does not exist", name);
	
	return it->second;
}


void PlanetarySystem::update(double dt, btDynamicsWorld* world, bool bullet)
{
	// TODO: This could be moved to load?
	if (states_now.empty())
	{
		states_now.resize(elements.size());
		// Load the initial positions and speeds
		for(size_t i = 0; i < elements.size(); i++)
		{
			states_now[i].pos = elements[i]->position_at_epoch;
			states_now[i].vel = elements[i]->velocity_at_epoch;
			states_now[i].mass = elements[i]->get_mass();
		}

		init_physics(world);
	}

	update_physics(dt, bullet);

}

void PlanetarySystem::init(btDynamicsWorld* world)
{
	propagator->bind_to(this);

}

static void load_body(SystemElement* body)
{
	logger->info("Loading body '{}'", body->name);
	if (body->config.has_surface)
	{
		body->renderer.rocky = new RockyPlanetRenderer();

		std::string script = AssetManager::load_string_raw(body->config.surface.script_path);

		body->renderer.rocky->load(script, body->config.surface.script_path_raw, body->config);
	}
	else
	{
		body->renderer.gas = new GasPlanetRenderer();
	}

	if (body->config.has_atmo)
	{
		body->renderer.atmo = new AtmosphereRenderer();
	}
}

static void unload_body(SystemElement* body)
{
	logger->info("Unloading body '{}'", body->name);
	if (body->config.has_surface && body->renderer.rocky != nullptr)
	{
		delete body->renderer.rocky;
		body->renderer.rocky = nullptr;
	}

	if (body->config.has_atmo && body->renderer.atmo != nullptr)
	{
		delete body->renderer.atmo;
		body->renderer.atmo = nullptr;
	}
}

void PlanetarySystem::update_render_body_rocky(SystemElement* body, glm::dvec3 body_pos, glm::dvec3 camera_pos, double t, double t0)
{
	bool moved = true;

	body->renderer.rocky->server->update(body->renderer.rocky->qtree);
	body->renderer.rocky->qtree.dirty = false;
	body->renderer.rocky->qtree.update(*body->renderer.rocky->server);

	if (moved)
	{
		// Build camera transform matrix, to get the relative camera pos
		glm::dmat4 rel_matrix = glm::dmat4(1.0);
		rel_matrix = rel_matrix * glm::inverse(body->build_rotation_matrix(t0, t));
		rel_matrix = glm::translate(rel_matrix, -body_pos);

		glm::dvec3 rel_camera_pos = rel_matrix * glm::dvec4(camera_pos, 1.0);
	


		glm::vec3 pos_nrm = (glm::vec3)glm::normalize(rel_camera_pos);
		PlanetSide side = body->renderer.rocky->qtree.get_planet_side(pos_nrm);
		glm::dvec2 offset = body->renderer.rocky->qtree.get_planet_side_offset(pos_nrm, side); 

		double altitude = body->renderer.rocky->server->get_height(pos_nrm, 1);

		double height = std::max(glm::length(rel_camera_pos) - body->config.radius - altitude, 1.0);

		height /= body->config.radius;

		// Simple formula that allows a great deal of customization
		double M = (double)body->config.surface.max_depth;
		double m = (double)body->config.surface.depth_for_unload;
		m = 0.0;			// TODO: Think this out
		double A = body->config.surface.coef_a;
		double B = body->config.surface.coef_b;
		double C = body->config.surface.coef_c;
		double x = height;

		// Formula: https://www.desmos.com/calculator/5xvcucqbrx
		double depthf = (M-1.0) * glm::exp(- (A * x) / C) + glm::pow(A, -glm::pow(x, B));

		int depthi = (int)std::round(depthf);
		size_t depth = glm::max(glm::min(depthi, body->config.surface.max_depth), 0);


		body->renderer.rocky->server->set_depth_for_unload(body->config.surface.depth_for_unload);

		body->renderer.rocky->qtree.set_wanted_subdivide(offset, side, depth);
		body->renderer.rocky->qtree.dirty = true;


		if (debug_drawer->debug_enabled)
		{
			// Add debug point at surface we are over
			debug_drawer->add_point(
				glm::inverse(rel_matrix) * glm::dvec4(glm::normalize(rel_camera_pos) * (altitude * 1.1 + body->config.radius * 1.01), 1.0),
				glm::vec3(1.0, 1.0, 1.0));

			//osp->game_state->universe.bt_world->debugDrawWorld();
		}
	}
}



void PlanetarySystem::update_render(glm::dvec3 camera_pos, float fov)
{

	for (size_t i = 0; i < elements.size(); i++)
	{
		
		// Set unloaded
		float prev_factor = elements[i]->dot_factor;
		elements[i]->dot_factor = elements[i]->get_dot_factor((float)glm::distance(camera_pos, states_now[i].pos), fov);



		if (prev_factor == 1.0f && elements[i]->dot_factor != 1.0f)
		{
			load_body(elements[i]);
		}

		if (prev_factor != 1.0f && elements[i]->dot_factor == 1.0f)
		{
			unload_body(elements[i]);
		}

		if (elements[i]->renderer.rocky != nullptr)
		{
			update_render_body_rocky(elements[i], states_now[i].pos, camera_pos, t, t0);
		}
	}
}

#include "propagator/RK4Propagator.h"

PlanetarySystem::PlanetarySystem(Universe* universe)
{
	this->universe = universe;

	states_now.resize(0);
	propagator = new RK4Propagator();

	name_to_index["__default"] = 0;
}


PlanetarySystem::~PlanetarySystem()
{
	delete propagator;

	// Remove physics stuff
	
	for(auto elem : elements)
	{
		if(elem->config.has_surface)
		{
			delete elem->rigid_body;
			delete elem->ground_shape;
		}

		delete elem;
	}
}

void PlanetarySystem::load(const cpptoml::table &root)
{
	t0 = root.get_qualified_as<double>("t").value_or(0);
	bt = 0; t = 0;

	auto toml_elements = root.get_table_array("element");
	if(!toml_elements) return;

	// Load all the elements first
	nbody_count = 0;

	for(const auto& toml_element : *toml_elements)
	{
		auto elem = new SystemElement();
		SerializeUtil::read_to(*toml_element, *elem);
		elements.push_back(elem);

		if(elem->nbody)
		{
			nbody_count++;
		}
	}

	// Sort by nbody tag for optimal simulation
	// We sadly can't use std::sort as it requires weak ordering
	std::vector<SystemElement*> ordered;
	size_t all = elements.size();
	while(ordered.size() != all)
	{
		for(auto it = elements.begin(); it != elements.end();)
		{
			if((*it)->nbody)
			{
				ordered.push_back((*it));
				it = elements.erase(it);
			}
			else
			{
				it++;
			}
		}
	}
	ordered.insert(ordered.end(), elements.begin(), elements.end());
	elements = ordered;

	// Create the name list
	for(size_t i = 0; i < elements.size(); i++)
	{
		elements[i]->index = i;

		if(elements[i]->name.empty()) continue;

		name_to_index[elements[i]->name] = i;
	}

}

SystemElement* PlanetarySystem::get_element(const std::string &name)
{
	auto it = name_to_index.find(name);
	logger->check(it != name_to_index.end());
	return elements[it->second];
}

CartesianState PlanetarySystem::get_element_state(const std::string &name, bool physics)
{
	auto it = name_to_index.find(name);
	logger->check(it != name_to_index.end());
	return get_element_state(it->second, physics);
}

CartesianState PlanetarySystem::get_element_state(size_t elem_id, bool physics)
{
	if(physics)
	{
		CartesianState st = states_now[elem_id];
		interp_pos(st);
		return st;
	}
	else
	{
		return states_now[elem_id];
	}
}

glm::dvec3 PlanetarySystem::get_element_position(const std::string &name)
{
	return get_element_state(name, false).pos;
}

glm::dvec3 PlanetarySystem::get_element_velocity(const std::string &name)
{
	return get_element_state(name, false).vel;
}

glm::dvec3 PlanetarySystem::interp_pos(size_t elem_id)
{
	double tdiff = t - bt;
	glm::dvec3 pos = states_now[elem_id].pos;
	glm::dvec3 vel = states_now[elem_id].vel;

	return pos + vel * tdiff;
}

void PlanetarySystem::interp_pos(CartesianState &st)
{
	double tdiff = t - bt;
	st.pos = st.pos + st.vel * tdiff;
}


