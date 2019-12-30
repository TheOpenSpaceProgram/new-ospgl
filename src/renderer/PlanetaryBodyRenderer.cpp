#include "PlanetaryBodyRenderer.h"
#include "../util/DebugDrawer.h"
#include "../universe/element/SystemElement.h"


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


void PlanetaryBodyRenderer::draw_debug(double t, CartesianState st, SystemElement* elem)
{
	if (elem->type == SystemElement::BARYCENTER)
	{ 
		// Barycenter
	}
	else if(elem->type == SystemElement::BODY)
	{
		float dot_factor = elem->as_body->dot_factor;
		if (dot_factor == 1.0f)
		{
			debug_drawer->add_point(st.pos, elem->as_body->config.far_color);
		}
		else
		{
			glm::dvec3 orbit_plane = elem->orbit.to_orbit_at(t).get_plane_normal();
			glm::dvec3 prograde = glm::normalize(st.vel);
			glm::dvec3 normal = glm::cross(prograde, orbit_plane);

			debug_drawer->add_line(st.pos, st.pos + elem->as_body->rotation_axis * elem->as_body->config.radius * 2.0, glm::vec3(1.0, 0.0, 0.0));
			debug_drawer->add_line(st.pos, st.pos - elem->as_body->rotation_axis * elem->as_body->config.radius * 2.0, glm::vec3(0.0, 0.0, 1.0));
			debug_drawer->add_line(st.pos, st.pos - glm::normalize(st.pos) * elem->as_body->config.radius * 1.5, glm::vec3(1.0, 0.5, 0.0));

			double oind_size = 2.5;

			debug_drawer->add_arrow(st.pos, st.pos + orbit_plane * elem->as_body->config.radius * oind_size, glm::vec3(0.0, 1.0, 1.0));
			debug_drawer->add_arrow(st.pos, st.pos + prograde * elem->as_body->config.radius * oind_size, glm::vec3(1.0, 1.0, 1.0));
			debug_drawer->add_arrow(st.pos, st.pos + normal * elem->as_body->config.radius * oind_size, glm::vec3(1.0, 1.0, 0.0));

		}
	}
	else
	{
		// Star
	}
}

PlanetaryBodyRenderer::PlanetaryBodyRenderer()
{
	rocky = nullptr;
	atmo = nullptr;
}


PlanetaryBodyRenderer::~PlanetaryBodyRenderer()
{
	if (rocky != nullptr)
	{
		delete rocky;
		rocky = nullptr;
	}

	if (atmo != nullptr)
	{
		delete atmo;
		atmo = nullptr;
	}
}

void RockyPlanetRenderer::load(const std::string& script, const std::string& script_path, PlanetConfig& config)
{
	if (server != nullptr)
	{
		delete server;
	}

	server = new PlanetTileServer(script, script_path, &config, config.surface.has_water);
}
