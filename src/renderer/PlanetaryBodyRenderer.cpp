#include "PlanetaryBodyRenderer.h"



void PlanetaryBodyRenderer::render(glm::dmat4 proj_view, glm::dmat4 model, float far_plane, glm::dvec3 camera_pos, PlanetConfig & config, double time)
{
	if (atmo != nullptr)
	{
		glm::dmat4 amodel = glm::translate(glm::dmat4(1.0f), -camera_pos);
		amodel = glm::scale(amodel, glm::dvec3(config.atmo.radius, config.atmo.radius, config.atmo.radius));
		float rel_radius = (float)(config.radius / config.atmo.radius);
		glm::vec3 cam_pos_relative = (glm::vec3)(camera_pos / config.atmo.radius);


		atmo->do_pass(proj_view, amodel, far_plane, rel_radius, cam_pos_relative,
			config.atmo.main_color, config.atmo.sunset_color, (float)config.atmo.exponent, (float)config.atmo.sunset_exponent);
	}
	if (rocky != nullptr)
	{
		rocky->renderer.render(*rocky->server, rocky->qtree, proj_view, model, far_plane, camera_pos, config, time);
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
