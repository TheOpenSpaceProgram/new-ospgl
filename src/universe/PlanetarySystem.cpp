#include "PlanetarySystem.h"
#include "../util/DebugDrawer.h"

glm::dvec3 compute_state(double t, double tol, double star_mass, PlanetaryBody* body, std::vector<glm::dvec3>* other_states)
{
	double parent_mass;
	glm::dvec3 offset = glm::dvec3(0.0, 0.0, 0.0);
	if (body->parent == nullptr)
	{
		parent_mass = star_mass;
	}
	else 
	{
		parent_mass = body->parent->config.mass;
		offset = (*other_states)[body->parent->index];
	}

	KeplerElements elems = body->orbit.to_elements_at(t, body->config.mass, parent_mass, tol);
	glm::dvec3 pos_rel = elems.get_position();

	return pos_rel + offset;
}


void PlanetarySystem::compute_states(double t, std::vector<glm::dvec3>& out, double tol)
{
	for (size_t i = 0; i < out.size(); i++)
	{
		out[i] = compute_state(t, tol, star_mass, &bodies[i], &out);
	}
}

void PlanetarySystem::compute_sois(double t)
{
	for (size_t i = 0; i < bodies.size(); i++)
	{
		double smajor_axis = bodies[i].orbit.to_orbit_at(t).smajor_axis;

		double parent_mass = star_mass;
		if (bodies[i].parent != nullptr)
		{
			parent_mass = bodies[i].parent->config.mass;
		}

		bodies[i].soi_radius = smajor_axis * pow(bodies[i].config.mass / parent_mass, 0.4);
	}
}

static glm::dmat4 build_body_rotation_matrix(PlanetaryBody* body, double t)
{
	glm::dmat4 rot_matrix = glm::mat4(1.0);

	double rot_angle = glm::radians(body->rotation_at_epoch + t * body->rotation_speed);
	rot_matrix = glm::rotate(rot_matrix, rot_angle, body->rotation_axis);
	// Align pole to rotation axis
	rot_matrix = rot_matrix * MathUtil::rotate_from_to(glm::dvec3(0.0, 1.0, 0.0), body->rotation_axis);


	return rot_matrix;
}

void PlanetarySystem::render_body(glm::dvec3 pos, PlanetaryBody* body, glm::dvec3 camera_pos, double t, 
	glm::dmat4 proj_view, float far_plane)
{
	glm::dmat4 model = glm::translate(glm::dmat4(1.0), -camera_pos + pos);
	model = glm::scale(model, glm::dvec3(body->config.radius));

	glm::dmat4 rot_matrix = build_body_rotation_matrix(body, t);
	
	model = model;

	glm::dvec3 camera_pos_relative = camera_pos - pos;
	glm::dvec3 light_dir = glm::normalize(pos);

	body->renderer.render(proj_view, model * rot_matrix, rot_matrix, far_plane, camera_pos_relative,
		body->config, t, light_dir, body->dot_factor);

	if (draw_debug)
	{
		body->renderer.draw_debug(pos, body, body->dot_factor);
	}
}



void PlanetarySystem::render(double t, int width, int height)
{
	double rt = t * 0.0 + 10.0;

	if (render_positions.size() == 0)
	{
		render_positions.resize(bodies.size());
	}

	float fov = glm::radians(80.0f);

	compute_states(t, render_positions, 1e-6);

	double dist = bodies[0].config.radius * 2.0f + t * bodies[0].config.radius * 0.0;

	glm::dvec3 camera_pos = render_positions[1] - glm::dvec3(cos(rt) * dist, 0.0, sin(rt) * dist);

	update_render(camera_pos, fov, t);


	// ~1 light year
	float far_plane = 1e16f;


	glm::dmat4 proj = glm::perspective((double)fov, (double)width / (double)height, 0.1, (double)far_plane);
	glm::dmat4 view = glm::lookAt(glm::dvec3(0.0, 0.0, 0.0), glm::dvec3(cos(rt), 0.0, sin(rt)), glm::dvec3(0.0, 1.0, 0.0));
	glm::dmat4 proj_view = proj * view;

	using BodyPositionPair = std::pair<PlanetaryBody*, glm::dvec3>;

	// Sort bodies by distance to camera to avoid weird atmospheres
	std::vector<BodyPositionPair> sorted;

	for (size_t i = 0; i < bodies.size(); i++)
	{
		sorted.push_back(std::make_pair(&bodies[i], render_positions[i]));
	}

	std::sort(sorted.begin(), sorted.end(), [camera_pos](BodyPositionPair a, BodyPositionPair b)
	{
		return glm::distance2(camera_pos, a.second) > glm::distance2(camera_pos, b.second);
	});

	for (size_t i = 0; i < sorted.size(); i++)
	{
		render_body(sorted[i].second, sorted[i].first, camera_pos, t, proj_view, far_plane);
	}

	if (draw_debug)
	{
		debug_drawer->add_point(glm::dvec3(0, 0, 0), glm::vec3(1.0, 1.0, 0.5));
	}

	glm::dmat4 c_model = glm::translate(glm::dmat4(1.0f), -camera_pos);
	// Don't forget to draw the debug shapes!
	debug_drawer->render(proj_view, c_model, far_plane);

}

static void load_body(PlanetaryBody* body)
{
	logger->info("Loading body '{}'", body->name);
	if (body->config.has_surface)
	{
		body->renderer.rocky = new RockyPlanetRenderer();

		std::string script = assets->loadString(body->config.surface.script_path);

		body->renderer.rocky->load(script, body->config);
	}
	else
	{

	}

	if (body->config.has_atmo)
	{
		body->renderer.atmo = new AtmosphereRenderer();
	}
}

static void unload_body(PlanetaryBody* body)
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
		rel_matrix = rel_matrix * glm::inverse(build_body_rotation_matrix(body, t));
		rel_matrix = glm::translate(rel_matrix, -body_pos);

		glm::dvec3 rel_camera_pos = rel_matrix * glm::dvec4(camera_pos, 1.0);
	


		glm::vec3 pos_nrm = (glm::vec3)glm::normalize(rel_camera_pos);
		PlanetSide side = body->renderer.rocky->qtree.get_planet_side(pos_nrm);
		glm::dvec2 offset = body->renderer.rocky->qtree.get_planet_side_offset(pos_nrm, side);

		double height = std::max(glm::length(rel_camera_pos) - body->config.radius -
			body->renderer.rocky->server->get_height(pos_nrm, 1), 1.0);

		height /= body->config.radius;
		double depthf = (body->config.surface.coef_a - (body->config.surface.coef_a * glm::log(height)
			/ ((glm::pow(height, 0.15) * body->config.surface.coef_b))) - 0.3 * height) * 0.4;

		size_t depth = (size_t)round(std::max(std::min(depthf, (double)body->config.surface.max_depth - 1.0), -1.0) + 1.0);


		body->renderer.rocky->qtree.set_wanted_subdivide(offset, side, depth);
		body->renderer.rocky->qtree.dirty = true;

		if (draw_debug)
		{
			// Add debug point at surface we are over
			debug_drawer->add_point(
				glm::inverse(rel_matrix) * glm::dvec4(glm::normalize(rel_camera_pos) * height * 1.01 * body->config.radius, 1.0),
				glm::vec3(1.0, 1.0, 1.0));
		}
	}
}

void PlanetarySystem::update_render(glm::dvec3 camera_pos, float fov, double t)
{

	for (size_t i = 0; i < bodies.size(); i++)
	{
		
		// Set unloaded
		float prev_factor = bodies[i].dot_factor;
		bodies[i].dot_factor = bodies[i].get_dot_factor((float)glm::distance(camera_pos, render_positions[i]), fov);

			

		if (prev_factor == 1.0f && bodies[i].dot_factor != 1.0f)
		{
			load_body(&bodies[i]);
		}

		if (prev_factor != 1.0f && bodies[i].dot_factor == 1.0f)
		{
			unload_body(&bodies[i]);
		}

		if (bodies[i].renderer.rocky != nullptr)
		{
			update_render_body_rocky(&bodies[i], render_positions[i], camera_pos, t);
		}
	}
}

PlanetarySystem::PlanetarySystem()
{
	render_positions.resize(0);
}


PlanetarySystem::~PlanetarySystem()
{
}
