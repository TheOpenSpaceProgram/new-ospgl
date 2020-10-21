#include "PlanetarySystem.h"
#include "../util/DebugDrawer.h"
#include <imgui/imgui.h>
#include "../physics/glm/BulletGlmCompat.h"
#include "../physics/ground/GroundShape.h"

glm::dvec3 PlanetarySystem::get_gravity_vector(glm::dvec3 p, StateVector* states)
{
	glm::dvec3 result = glm::dvec3(0, 0, 0);
	for(size_t i = 0; i < states->size(); i++)
	{
		double mass = elements[i].get_real_mass();
		
		if(mass > 0)
		{
			glm::dvec3 pos = states->at(i).pos;
			glm::dvec3 diff = pos - p;

			double dist2 = glm::length2(diff);

			glm::dvec3 diffn = diff / glm::sqrt(dist2);
			double am = (G * mass) / dist2;
			result += diffn * am;
		}		
	}	

	return result;
}

CartesianState compute_state(double t0, double t, double tol, 
	SystemElement* body, std::vector<CartesianState>* other_states)
{
	if (body->type == SystemElement::STAR)
	{
		return CartesianState(glm::dvec3(0.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, 0.0));
	}

	double parent_mass;
	glm::dvec3 offset = glm::dvec3(0.0, 0.0, 0.0);
	glm::dvec3 vel_offset = glm::dvec3(0.0, 0.0, 0.0);

	parent_mass = body->parent->get_mass(body->is_primary);
	offset = (*other_states)[body->parent->index].pos;
	vel_offset = (*other_states)[body->parent->index].vel;

	if (body->is_primary && body->parent->type == SystemElement::BARYCENTER)
	{
		CartesianState moon = (*other_states)[body->parent->as_barycenter->secondary->index];
		// Invert it across the barycenter

		double moon_radius = glm::length(moon.pos - offset);

		glm::dvec3 ov = glm::normalize(moon.pos - offset);
		moon.pos = offset - ov * body->barycenter_radius;

		// Angular velocity is the same, so linear velocity scales 
		// lin_vel = radius * ang_vel
		// lin_vel' = radius' * ang_vel
		// lin_vel' = lin_vel * (radius' / radius) (Dividing the two)
		double lin_vel = glm::length(moon.vel - vel_offset);
		double radius = moon_radius;
		double radius_p = glm::length(body->barycenter_radius);

		double lin_vel_p = lin_vel * (radius_p / radius);

		// Opposite direction, scaled velocity
		moon.vel = -glm::normalize(moon.vel - vel_offset) * lin_vel_p + vel_offset;

		return moon;
	}
	else
	{

		KeplerElements elems = body->orbit.to_elements_at(t0, t, body->get_mass(), parent_mass, tol);
		CartesianState st = elems.get_cartesian(parent_mass, body->get_mass());

		st.pos += offset;
		st.vel += vel_offset;
		return st;
	}


}

glm::dvec3 compute_pos(double t0, double t, double tol,
	SystemElement* body, std::vector<glm::dvec3>* other_positions)
{
	if (body->type == SystemElement::STAR)
	{
		return glm::dvec3(0.0, 0.0, 0.0);
	}

	double parent_mass;
	glm::dvec3 offset = glm::dvec3(0.0, 0.0, 0.0);
	parent_mass = body->parent->get_mass(body->is_primary);
	offset = (*other_positions)[body->parent->index];

	if (body->is_primary && body->parent->type == SystemElement::BARYCENTER)
	{
		glm::dvec3 moon = (*other_positions)[body->parent->as_barycenter->secondary->index];
		// Invert it across the barycenter
		glm::dvec3 ov = glm::normalize(moon - offset);
		moon = offset - ov * body->barycenter_radius;
		return moon;
	}
	else
	{

		KeplerElements elems = body->orbit.to_elements_at(t0, t, body->get_mass(), parent_mass, tol);
		glm::dvec3 st = elems.get_position();
		st += offset;

		return st;
	}
}


void PlanetarySystem::compute_states(double t0, double t, std::vector<CartesianState>& out, double tol)
{
	for (size_t i = 0; i < out.size(); i++)
	{
		out[i] = compute_state(t0, t, tol, &elements[i], &out);
	}
}

void PlanetarySystem::compute_positions(double t0, double t, std::vector<glm::dvec3>& out, double tol)
{
	for (size_t i = 0; i < out.size(); i++)
	{
		out[i] = compute_pos(t0, t, tol, &elements[i], &out);
	}
}


void PlanetarySystem::compute_sois(double t0, double t)
{
	elements[0].soi_radius = std::numeric_limits<double>::infinity();

	for (size_t i = 1; i < elements.size(); i++)
	{
		double smajor_axis = elements[i].orbit.to_orbit_at(t0, t).smajor_axis;

		double parent_mass = elements[i].parent->get_mass();

		elements[i].soi_radius = smajor_axis * pow(elements[i].get_mass() / parent_mass, 0.4);
	}
}

void PlanetarySystem::render_body(CartesianState state, SystemElement* body, glm::dvec3 camera_pos, double t, 
	glm::dmat4 proj_view, float far_plane)
{

	glm::dvec3 camera_pos_relative = camera_pos - state.pos;
	glm::dvec3 light_dir = glm::normalize(state.pos);

	if (body->type == SystemElement::BODY)
	{
		glm::dmat4 model = glm::translate(glm::dmat4(1.0), -camera_pos + state.pos);
		model = glm::scale(model, glm::dvec3(body->as_body->config.radius));

		glm::dmat4 rot_matrix = body->as_body->build_rotation_matrix(t0, t);

		body->as_body->renderer.deferred(proj_view, model * rot_matrix, rot_matrix, far_plane, camera_pos_relative,
			body->as_body->config, t, light_dir, body->as_body->dot_factor);
	}

	if (debug_drawer->debug_enabled)
	{
		body->as_body->renderer.draw_debug(t0, t, state, body);
	}
}

void PlanetarySystem::render_body_atmosphere(CartesianState state, SystemElement * body, glm::dvec3 camera_pos, double t, glm::dmat4 proj_view, float far_plane)
{
	glm::dvec3 camera_pos_relative = camera_pos - state.pos;
	glm::dvec3 light_dir = glm::normalize(state.pos);

	if (body->type == SystemElement::BODY)
	{
		body->as_body->renderer.forward(proj_view, camera_pos_relative, body->as_body->config, far_plane, light_dir);
	}
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
		update_render(camera_pos, fov, t);
	}

	glm::dmat4 proj_view = cu.proj_view;
	glm::dmat4 c_model = cu.c_model;
	float far_plane = cu.far_plane;

	for (size_t i = 0; i < elements.size(); i++)
	{
		render_body(states_now[i], &elements[i], camera_pos, t, proj_view, far_plane);
	}

	if (debug_drawer->debug_enabled)
	{
		// 1 AU
		double axis_length = 149597900000;
		debug_drawer->add_point(glm::dvec3(0, 0, 0), glm::vec3(1.0, 1.0, 0.5));

		debug_drawer->add_line(glm::dvec3(0, 0, 0), glm::dvec3(1, 0, 0) * axis_length, glm::vec3(1.0, 0.0, 0.0));
		debug_drawer->add_line(glm::dvec3(0, 0, 0), glm::dvec3(0, 1, 0) * axis_length, glm::vec3(0.0, 1.0, 0.0));
		debug_drawer->add_line(glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 1) * axis_length, glm::vec3(0.0, 0.0, 1.0));

		// Add debug orbits
		for (size_t i = 1; i < elements.size(); i++)
		{
			glm::dvec3 origin = glm::dvec3(0, 0, 0);
			int verts = 1024;
			if (elements[i].parent != nullptr)
			{
				origin = states_now[elements[i].parent->index].pos;
				verts = 128;
			}

			glm::vec3 col;

			if (elements[i].type == SystemElement::BARYCENTER)
			{
				col = elements[i].as_barycenter->primary->as_body->config.far_color;
			}
			else
			{
				col = elements[i].as_body->config.far_color;
			}

			bool striped = false;
			if (elements[i].is_primary)
			{
				striped = true;
			}

			debug_drawer->add_orbit(origin, elements[i].orbit.to_orbit_at(t0, t), col, striped, verts);
		}
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
		sorted.push_back(std::make_pair(&elements[i], states_now[i]));
	}

	std::sort(sorted.begin(), sorted.end(), [camera_pos](BodyPositionPair a, BodyPositionPair b)
	{
		return glm::distance2(camera_pos, a.second.pos) > glm::distance2(camera_pos, b.second.pos);
	});

	for (size_t i = 0; i < sorted.size(); i++)
	{
		render_body_atmosphere(sorted[i].second, sorted[i].first, camera_pos, t, proj_view, far_plane);
	}
}


void PlanetarySystem::update_physics(double dt, bool bullet)
{
	StateVector* v;
	double tnow;
	if (bullet)
	{
		tnow = bt;
		v = &bullet_states;
	}
	else
	{
		tnow = t;
		v = &states_now;
	}

	compute_states(t0, tnow + dt * timewarp, *v, 1e-12);


	if (bullet)
	{
		bt += dt * timewarp;
		// Give data to colliders
		for(size_t i = 0; i < elements.size(); i++)
		{
			SystemElement& elem = elements[i];

			if(elem.type == SystemElement::BODY)
			{
				PlanetaryBody* as_body = elem.as_body;
			
				btTransform tform = btTransform::getIdentity();
				tform.setOrigin(to_btVector3(bullet_states[i].pos));
				glm::dmat4 mat = as_body->build_rotation_matrix(t0, bt);
				glm::dquat quat = glm::dquat(mat);

				tform.setRotation(to_btQuaternion(quat));

				as_body->rigid_body->setWorldTransform(tform);	
			}
		}

	}
	else
	{ 
		t += dt * timewarp;
	}
	
}

void PlanetarySystem::init_physics(btDynamicsWorld* world)
{
	bt = t;
	
	// Create the colliders and rigidbodies

	for(size_t i = 0; i < elements.size(); i++)
	{
		SystemElement& elem = elements[i];

		if(elem.type == SystemElement::BODY)
		{
			PlanetaryBody* as_body = elem.as_body;

			as_body->ground_shape = new GroundShape(as_body);
			as_body->rigid_body = new btRigidBody(1000000000.0, nullptr, as_body->ground_shape, btVector3(0, 0, 0));
			as_body->rigid_body->setCollisionFlags(as_body->rigid_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			as_body->rigid_body->setFriction(1.0);

			as_body->rigid_body->setRestitution(1.0);
			as_body->ground_shape->setMargin(2.0);
			as_body->rigid_body->setActivationState(DISABLE_DEACTIVATION);

			world->addRigidBody(as_body->rigid_body);
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
	if (bullet_states.size() == 0)
	{
		bullet_states.resize(elements.size());
	}

	if (states_now.size() == 0)
	{
		init_physics(world);
		states_now.resize(elements.size());
	}

	update_physics(dt, bullet);
	
}

void PlanetarySystem::init(btDynamicsWorld* world)
{
	propagator->initialize(this, elements.size());

}

static void load_body(SystemElement* body)
{
	logger->info("Loading body '{}'", body->name);
	if (body->as_body->config.has_surface)
	{
		body->as_body->renderer.rocky = new RockyPlanetRenderer();

		std::string script = assets->load_string_raw(body->as_body->config.surface.script_path);

		body->as_body->renderer.rocky->load(script, body->as_body->config.surface.script_path_raw, body->as_body->config);
	}
	else
	{

	}

	if (body->as_body->config.has_atmo)
	{
		body->as_body->renderer.atmo = new AtmosphereRenderer();
	}
}

static void unload_body(SystemElement* body)
{
	logger->info("Unloading body '{}'", body->name);
	if (body->as_body->config.has_surface && body->as_body->renderer.rocky != nullptr)
	{
		delete body->as_body->renderer.rocky;
		body->as_body->renderer.rocky = nullptr;
	}

	if (body->as_body->config.has_atmo && body->as_body->renderer.atmo != nullptr)
	{
		delete body->as_body->renderer.atmo;
		body->as_body->renderer.atmo = nullptr;
	}
}

void PlanetarySystem::update_render_body_rocky(PlanetaryBody* body, glm::dvec3 body_pos, glm::dvec3 camera_pos, double t)
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
		}
	}
}



void PlanetarySystem::update_render(glm::dvec3 camera_pos, float fov, double t)
{

	for (size_t i = 0; i < elements.size(); i++)
	{
		
		if (elements[i].type == SystemElement::BODY)
		{

			// Set unloaded
			float prev_factor = elements[i].as_body->dot_factor;
			elements[i].as_body->dot_factor = elements[i].as_body->get_dot_factor((float)glm::distance(camera_pos, states_now[i].pos), fov);



			if (prev_factor == 1.0f && elements[i].as_body->dot_factor != 1.0f)
			{
				load_body(&elements[i]);
			}

			if (prev_factor != 1.0f && elements[i].as_body->dot_factor == 1.0f)
			{
				unload_body(&elements[i]);
			}

			if (elements[i].as_body->renderer.rocky != nullptr)
			{
				update_render_body_rocky(elements[i].as_body, states_now[i].pos, camera_pos, t);
			}
		}
	}
}

#include "propagator/RK4Interpolated.h"

PlanetarySystem::PlanetarySystem(Universe* universe)
{
	this->universe = universe;

	states_now.resize(0);
	propagator = new RK4Interpolated();
	timewarp = 1.0;
	t = 0.0;
}


PlanetarySystem::~PlanetarySystem()
{
	delete propagator;

	// Remove physics stuff
	
	for(size_t i = 0; i < elements.size(); i++)
	{
		SystemElement& elem = elements[i];

		if(elem.type == SystemElement::BODY)
		{
			PlanetaryBody* as_body = elem.as_body;
			delete as_body->rigid_body;
			delete as_body->ground_shape;
		}
	}
}
