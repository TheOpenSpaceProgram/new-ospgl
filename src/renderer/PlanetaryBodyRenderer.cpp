#include "PlanetaryBodyRenderer.h"
#include "../util/DebugDrawer.h"
#include "../universe/body/PlanetaryBody.h"


void PlanetaryBodyRenderer::render(glm::dmat4 proj_view, glm::dmat4 model, glm::dmat4 rotation_matrix,
	double far_plane, glm::dvec3 camera_pos, 
	PlanetConfig & config, double time, glm::vec3 light_dir, float dot_factor)
{
	if (atmo != nullptr)
	{
		glm::dmat4 amodel = glm::translate(glm::dmat4(1.0f), -camera_pos);
		amodel = glm::scale(amodel, glm::dvec3(config.atmo.radius, config.atmo.radius, config.atmo.radius));
		float rel_radius = (float)(config.radius / config.atmo.radius);
		glm::vec3 cam_pos_relative = (glm::vec3)(camera_pos / config.atmo.radius);


		atmo->do_pass(proj_view, amodel, (float)far_plane, cam_pos_relative, config, light_dir);
	}
	if (rocky != nullptr)
	{
		// Normal matrix is used to transform normals
		glm::dmat4 normal_matrix = glm::transpose(glm::inverse(rotation_matrix));

		// We have to give the renderer the rotation matrix so atmosphere
		// can be rendered properly
		rocky->renderer.render(*rocky->server, rocky->qtree, proj_view, model, rotation_matrix, normal_matrix, (float)far_plane, camera_pos, config, time, light_dir);
	}
}


void PlanetaryBodyRenderer::draw_debug(double t, CartesianState st, PlanetaryBody* body, float dot_factor)
{
	if (dot_factor == 1.0f)
	{
		debug_drawer->add_point(st.pos, body->config.far_color);
	}
	else
	{
		glm::dvec3 orbit_plane = body->orbit.to_orbit_at(t).get_plane_normal();
		glm::dvec3 prograde = glm::normalize(st.vel);
		glm::dvec3 normal = glm::cross(prograde, orbit_plane);

		debug_drawer->add_line(st.pos, st.pos + body->rotation_axis * body->config.radius * 2.0, glm::vec3(1.0, 0.0, 0.0));
		debug_drawer->add_line(st.pos, st.pos - body->rotation_axis * body->config.radius * 2.0, glm::vec3(0.0, 0.0, 1.0));
		debug_drawer->add_line(st.pos, st.pos - glm::normalize(st.pos) * body->config.radius * 1.5, glm::vec3(1.0, 0.5, 0.0));

		logger->info("dot(rot, sun) = {}", glm::dot(body->rotation_axis, -glm::normalize(st.pos)));

		double oind_size = 2.5;

		debug_drawer->add_arrow(st.pos, st.pos + orbit_plane * body->config.radius * oind_size, glm::vec3(0.0, 1.0, 1.0));
		debug_drawer->add_arrow(st.pos, st.pos + prograde * body->config.radius * oind_size, glm::vec3(1.0, 1.0, 1.0));
		debug_drawer->add_arrow(st.pos, st.pos + normal * body->config.radius * oind_size, glm::vec3(1.0, 1.0, 0.0));

	}
}

PlanetaryBodyRenderer::PlanetaryBodyRenderer()
{
	rocky = nullptr;
	atmo = nullptr;
}


PlanetaryBodyRenderer::~PlanetaryBodyRenderer()
{
}

void RockyPlanetRenderer::load(std::string script, PlanetConfig& config)
{
	if (server != nullptr)
	{
		delete server;
	}

	server = new PlanetTileServer(script, &config, config.surface.seed, config.surface.interp, config.surface.has_water);
}
