#include "PlanetarySystem.h"
#include "../util/DebugDrawer.h"
#include <imgui/imgui.h>


CartesianState compute_state(double t, double tol, double star_mass, 
	SystemElement* body, std::vector<CartesianState>* other_states)
{
	double parent_mass;
	glm::dvec3 offset = glm::dvec3(0.0, 0.0, 0.0);
	glm::dvec3 vel_offset = glm::dvec3(0.0, 0.0, 0.0);
	if (body->parent == nullptr)
	{
		parent_mass = star_mass;
	}
	else 
	{
		parent_mass = body->parent->get_mass(body->is_primary);
		offset = (*other_states)[body->parent->index + 1].pos;
		vel_offset = (*other_states)[body->parent->index + 1].vel;
	}

	if (body->is_primary && body->parent->is_barycenter)
	{
		CartesianState moon = (*other_states)[body->parent->barycenter_secondary->index + 1];
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

glm::dvec3 compute_pos(double t, double tol, double star_mass,
	SystemElement* body, std::vector<glm::dvec3>* other_positions)
{
	double parent_mass;
	glm::dvec3 offset = glm::dvec3(0.0, 0.0, 0.0);
	if (body->parent == nullptr)
	{
		parent_mass = star_mass;
	}
	else
	{
		parent_mass = body->parent->get_mass(body->is_primary);
		offset = (*other_positions)[body->parent->index + 1];
	}

	if (body->is_primary && body->parent->is_barycenter)
	{
		glm::dvec3 moon = (*other_positions)[body->parent->barycenter_secondary->index + 1];
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
	out[0] = CartesianState({ 0, 0, 0 }, { 0, 0, 0 });
	for (size_t i = 1; i < out.size(); i++)
	{
		out[i] = compute_state(t, tol, star_mass, &elements[i - 1], &out);
	}
}

void PlanetarySystem::compute_positions(double t, std::vector<glm::dvec3>& out, double tol)
{
	out[0] = glm::dvec3(0.0, 0.0, 0.0);

	for (size_t i = 1; i < out.size(); i++)
	{
		out[i] = compute_pos(t, tol, star_mass, &elements[i - 1], &out);
	}
}


void PlanetarySystem::compute_sois(double t)
{
	for (size_t i = 0; i < elements.size(); i++)
	{
		double smajor_axis = elements[i].orbit.to_orbit_at(t).smajor_axis;

		double parent_mass = star_mass;
		if (elements[i].parent != nullptr)
		{
			parent_mass = elements[i].parent->get_mass();
		}

		elements[i].soi_radius = smajor_axis * pow(elements[i].get_mass() / parent_mass, 0.4);
	}
}

void PlanetarySystem::render_body(CartesianState state, SystemElement* body, glm::dvec3 camera_pos, double t, 
	glm::dmat4 proj_view, float far_plane)
{
	double M_TO_AU = 1.0 / 149597900000.0;



	glm::dvec3 camera_pos_relative = camera_pos - state.pos;
	glm::dvec3 light_dir = glm::normalize(state.pos);

	if (!body->is_barycenter)
	{
		glm::dmat4 model = glm::translate(glm::dmat4(1.0), -camera_pos + state.pos);
		model = glm::scale(model, glm::dvec3(body->as_body->config.radius));

		glm::dmat4 rot_matrix = body->as_body->build_rotation_matrix(t);

		body->as_body->renderer.render(proj_view, model * rot_matrix, rot_matrix, far_plane, camera_pos_relative,
			body->as_body->config, t, light_dir, body->as_body->dot_factor);
	}

	if (debug_drawer->debug_enabled)
	{
		body->as_body->renderer.draw_debug(t, state, body);
	}
}

#include "../util/InputUtil.h"

static double zoom = 1.0;

void PlanetarySystem::render(int width, int height)
{
	float fov = glm::radians(60.0f);

	auto[camera_pos, camera_dir] = camera.get_camera_pos_dir(t, vessels[0].state.pos, star_radius, states_now, elements);

	update_render(camera_pos, fov, t);


	// ~1 light year
	float far_plane = 1e16f;


	glm::dmat4 proj = glm::perspective((double)fov, (double)width / (double)height, 0.1, (double)far_plane);
	glm::dmat4 view = glm::lookAt(glm::dvec3(0.0, 0.0, 0.0), camera_dir, glm::dvec3(0.0, 1.0, 0.0));
	glm::dmat4 proj_view = proj * view;

	using BodyPositionPair = std::pair<SystemElement*, CartesianState>;

	// Sort bodies by distance to camera to avoid weird atmospheres
	std::vector<BodyPositionPair> sorted;

	for (size_t i = 0; i < elements.size(); i++)
	{
		sorted.push_back(std::make_pair(&elements[i], states_now[i + 1]));
	}

	std::sort(sorted.begin(), sorted.end(), [camera_pos](BodyPositionPair a, BodyPositionPair b)
	{
		return glm::distance2(camera_pos, a.second.pos) > glm::distance2(camera_pos, b.second.pos);
	});

	for (size_t i = 0; i < sorted.size(); i++)
	{
		render_body(sorted[i].second, sorted[i].first, camera_pos, t, proj_view, far_plane);
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
		for (size_t i = 0; i < elements.size(); i++)
		{
			glm::dvec3 origin = glm::dvec3(0, 0, 0);
			int verts = 1024;
			if (elements[i].parent != nullptr)
			{
				origin = states_now[elements[i].parent->index + 1].pos;
				verts = 128;
			}

			glm::vec3 col;

			if (elements[i].is_barycenter)
			{
				col = elements[i].barycenter_primary->as_body->config.far_color;
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

void PlanetarySystem::render_debug(int width, int height)
{
	float fov = glm::radians(60.0f);
	auto[camera_pos, camera_dir] = camera.get_camera_pos_dir(t, vessels[0].state.pos, star_radius, states_now, elements);
	// ~1 light year
	float far_plane = 1e16f;


	glm::dmat4 proj = glm::perspective((double)fov, (double)width / (double)height, 0.1, (double)far_plane);
	glm::dmat4 view = glm::lookAt(glm::dvec3(0.0, 0.0, 0.0), camera_dir, glm::dvec3(0.0, 1.0, 0.0));
	glm::dmat4 proj_view = proj * view;

	glm::dmat4 c_model = glm::translate(glm::dmat4(1.0), -camera_pos);

	// Don't forget to draw the debug shapes!
	debug_drawer->render(proj_view, c_model, far_plane);

}

static double pt;
static int factor;

void PlanetarySystem::update_physics(double dt)
{
	compute_states(t + dt * timewarp, states_now, 1e-6);

	//int factor = 10000;
	double sub_dt = dt * factor;
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
		propagator->prepare(sub_t, sub_dt, physics_pos);
		size_t closest = propagator->propagate(&vessels[0]);

		vessels[0].simulate(elements, physics_pos, star_radius, closest, sub_dt);
		

		it++;
	}

	debug_drawer->debug_enabled = prev_debug;

	pt -= dt * timewarp;

	if (pt < 0.0)
	{
		pts.push_back(vessels[0].state.pos - states_now[3].pos);
		pt = 60.0 * 60.0;
	}

	for (size_t i = 0; i < pts.size(); i++)
	{
		debug_drawer->add_point(pts[i] + states_now[3].pos, glm::vec3(1.0, 1.0, 1.0));
	}


	vessels[0].draw_debug();


	t += dt * timewarp;
}

void PlanetarySystem::init_physics()
{
	pt = 0.0;
	factor = 1000;

}


void PlanetarySystem::update(double dt)
{
	dt = 0.01;

	if (states_now.size() == 0)
	{
		init_physics();
		states_now.resize(elements.size() + 1);
	}

	update_physics(dt);



	ImGui::Begin("Camera Focus");
	
	ImGui::InputInt("Focus", &camera.focus_index);

	if (camera.focus_index < -1)
	{
		camera.focus_index = -1;
	}

	if (camera.focus_index > (int)elements.size())
	{
		camera.focus_index = (int)elements.size();
	}

	if (ImGui::Button("Position Vessel"))
	{
		vessels[0].state = states_now[4];
		double r = elements[3].as_body->config.radius;
		vessels[0].state.pos += glm::dvec3(r * 0.8, 4000 * 1e3, 0.0);
		vessels[0].state.vel += glm::dvec3(0.0, 0.0, 8250.5);
		camera.focus_index = -1;
		camera.distance = 5.0;
	}

	ImGui::InputInt("Timestep: ", &factor);


	ImGui::End();

	//camera.distance = 1000000000000.0;
	camera.update(dt);

	
}

void PlanetarySystem::init()
{
	propagator->initialize(this, elements.size());
	vessels.push_back(Vessel());
	vessels[0].state.pos = glm::dvec3(0.0, 0.0, 0.0);
}

static void load_body(SystemElement* body)
{
	logger->info("Loading body '{}'", body->name);
	if (body->as_body->config.has_surface)
	{
		body->as_body->renderer.rocky = new RockyPlanetRenderer();

		std::string script = assets->loadString(body->as_body->config.surface.script_path);

		body->as_body->renderer.rocky->load(script, body->as_body->config);
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
		
		if (!elements[i].is_barycenter)
		{

			// Set unloaded
			float prev_factor = elements[i].as_body->dot_factor;
			elements[i].as_body->dot_factor = elements[i].as_body->get_dot_factor((float)glm::distance(camera_pos, states_now[i + 1].pos), fov);



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
				update_render_body_rocky(elements[i].as_body, states_now[i + 1].pos, camera_pos, t);
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
