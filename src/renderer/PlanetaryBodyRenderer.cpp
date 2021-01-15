#include "PlanetaryBodyRenderer.h"
#include "../util/DebugDrawer.h"
#include "../universe/element/SystemElement.h"


void PlanetaryBodyRenderer::deferred(glm::dmat4 proj_view, glm::dmat4 model, glm::dmat4 rotation_matrix,
									 double far_plane, glm::dvec3 camera_pos,
									 ElementConfig & config, double time, glm::vec3 light_dir, float dot_factor) const
{

	glm::dmat4 normal_matrix = glm::transpose(glm::inverse(rotation_matrix));

	if (rocky != nullptr)
	{
		// We have to give the renderer the rotation matrix so atmosphere
		// can be rendered properly
		rocky->renderer.render(*rocky->server, rocky->qtree, proj_view, model,
			rotation_matrix, normal_matrix, (float)far_plane, camera_pos, config, time, light_dir);
	}
	else if(gas != nullptr)
	{
		gas->render(proj_view, model, rotation_matrix, normal_matrix, (float)far_plane, camera_pos, config, time);
	}
}

void PlanetaryBodyRenderer::forward(glm::dmat4 proj_view, glm::dvec3 camera_pos,
									ElementConfig& config, double far_plane, glm::vec3 light_dir) const
{
	if (atmo != nullptr)
	{
		glm::dmat4 amodel = glm::translate(glm::dmat4(1.0f), -camera_pos);
		amodel = glm::scale(amodel, glm::dvec3(config.atmo.radius, config.atmo.radius, config.atmo.radius));
		glm::vec3 cam_pos_relative = (glm::vec3)(camera_pos / config.atmo.radius);


		atmo->do_pass(proj_view, amodel, (float)far_plane, cam_pos_relative, config, light_dir);
	}
}


void PlanetaryBodyRenderer::draw_debug(double t0, double t, CartesianState st, SystemElement* elem)
{
	float dot_factor = elem->dot_factor;
	if (dot_factor == 1.0f)
	{
		debug_drawer->add_point(st.pos, elem->config.far_color);
	}
	else
	{
	}
}

PlanetaryBodyRenderer::PlanetaryBodyRenderer()
{
	rocky = nullptr;
	atmo = nullptr;
}


PlanetaryBodyRenderer::~PlanetaryBodyRenderer()
{
	delete rocky;
	rocky = nullptr;

	delete atmo;
	atmo = nullptr;
}

