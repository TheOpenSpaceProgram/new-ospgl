#include "PlanetarySystem.h"
#include "../util/DebugDrawer.h"
#include <imgui/imgui.h>
#include "ReferenceFrame.h"

CartesianState compute_state(double t, double tol, 
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
		glm::dvec3 ov = glm::normalize(moon.pos - offset);
		moon.pos = offset - ov * body->barycenter_radius;
		// Scale velocity
		moon.vel = -glm::normalize(moon.vel - vel_offset) * body->barycenter_radius + vel_offset;

		moon.vel = vel_offset;

		return moon;
	}
	else
	{

		KeplerElements elems = body->orbit.to_elements_at(t, body->get_mass(), parent_mass, tol);
		CartesianState st = elems.get_cartesian(parent_mass, body->get_mass());

		st.pos += offset;
		st.vel += vel_offset;
		return st;
	}


}

glm::dvec3 compute_pos(double t, double tol,
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

		KeplerElements elems = body->orbit.to_elements_at(t, body->get_mass(), parent_mass, tol);
		glm::dvec3 st = elems.get_position();
		st += offset;

		return st;
	}
}


void PlanetarySystem::compute_states(double t, std::vector<CartesianState>& out, double tol)
{
	for (size_t i = 0; i < out.size(); i++)
	{
		out[i] = compute_state(t, tol, &elements[i], &out);
	}
}

void PlanetarySystem::compute_positions(double t, std::vector<glm::dvec3>& out, double tol)
{
	for (size_t i = 0; i < out.size(); i++)
	{
		out[i] = compute_pos(t, tol, &elements[i], &out);
	}
}


void PlanetarySystem::compute_sois(double t)
{
	elements[0].soi_radius = std::numeric_limits<double>::infinity();

	for (size_t i = 1; i < elements.size(); i++)
	{
		double smajor_axis = elements[i].orbit.to_orbit_at(t).smajor_axis;

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

		glm::dmat4 rot_matrix = body->as_body->build_rotation_matrix(t);

		body->as_body->renderer.deferred(proj_view, model * rot_matrix, rot_matrix, far_plane, camera_pos_relative,
			body->as_body->config, t, light_dir, body->as_body->dot_factor);
	}

	if (debug_drawer->debug_enabled)
	{
		body->as_body->renderer.draw_debug(t, state, body);
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


void PlanetarySystem::deferred_pass(CameraUniforms & cu)
{
	float fov = glm::radians(60.0f);

	glm::dvec3 camera_pos = cu.cam_pos;

	update_render(camera_pos, fov, t);


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

			debug_drawer->add_orbit(origin, elements[i].orbit.to_orbit_at(t), col, striped, verts);
		}
	}
}

void PlanetarySystem::forward_pass(CameraUniforms & cu)
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

static double pt;
static int factor;

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

	compute_states(tnow + dt * timewarp, *v, 1e-6);


	if (!bullet)
	{
		// TODO: Fix this mess
		//int factor = 10000;
		/*double sub_dt = dt * factor;
		if (timewarp < factor)
		{
			sub_dt = dt * timewarp;
		}

		bool prev_debug = debug_drawer->debug_enabled;

		int it = 0;
		for (double sub_t = t; sub_t < t + dt * timewarp; sub_t += sub_dt)
		{
			// We only draw debug at sub_t = 0
			if (it != 0)
			{
				debug_drawer->debug_enabled = false;
			}

			// Propagate vessels
			//propagator->prepare(sub_t, sub_dt, physics_pos);
			//size_t closest = propagator->propagate(&vessels[0]);

			//vessels[0].simulate(elements, physics_pos, closest, sub_dt);


			it++;
		}

		debug_drawer->debug_enabled = prev_debug;

		pt -= dt * timewarp;


		glm::dvec3 pp;



		//vessels[0].draw_debug();
		*/
	}

	if (bullet)
	{
		bt += dt * timewarp;
	}
	else
	{ 
		t += dt * timewarp;
	}
	
}

void PlanetarySystem::init_physics(btDynamicsWorld* world)
{
	bt = t;

	pt = 0;
	factor = 1000;

}

size_t PlanetarySystem::get_element_index_from_name(const std::string& name)
{
	auto it = name_to_index.find(name);
	// TODO: Maybe make this not important?
	logger->check_important(it != name_to_index.end(), "Tried to access by name a element which does not exist");
	
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
		rel_matrix = rel_matrix * glm::inverse(body->build_rotation_matrix(t));
		rel_matrix = glm::translate(rel_matrix, -body_pos);

		glm::dvec3 rel_camera_pos = rel_matrix * glm::dvec4(camera_pos, 1.0);
	


		glm::vec3 pos_nrm = (glm::vec3)glm::normalize(rel_camera_pos);
		PlanetSide side = body->renderer.rocky->qtree.get_planet_side(pos_nrm);
		glm::dvec2 offset = body->renderer.rocky->qtree.get_planet_side_offset(pos_nrm, side); 

		double altitude = body->renderer.rocky->server->get_height(pos_nrm, 1);

		double height = std::max(glm::length(rel_camera_pos) - body->config.radius - altitude, 1.0);

		height /= body->config.radius;
		double depthf = (body->config.surface.coef_a - (body->config.surface.coef_a * glm::log(height)
			/ ((glm::pow(height, 0.15) * body->config.surface.coef_b))) - 0.3 * height) * 0.4;

		size_t depth = (size_t)round(std::max(std::min(depthf, (double)body->config.surface.max_depth - 1.0), -1.0) + 1.0);

		// 2 takes about 120Mb of memory so it's not too high
		// and it's the usual orbital level of detail
		body->renderer.rocky->server->set_depth_for_unload(2);

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

PlanetarySystem::PlanetarySystem()
{
	states_now.resize(0);
	propagator = new RK4Interpolated();
	timewarp = 1.0;
	t = 0.0;
}


PlanetarySystem::~PlanetarySystem()
{
	delete propagator;
}
