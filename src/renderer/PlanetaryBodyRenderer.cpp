#include "PlanetaryBodyRenderer.h"
#include "../util/DebugDrawer.h"
#include "../universe/element/SystemElement.h"


void PlanetaryBodyRenderer::deferred(const PlanetRenderer::PlanetRenderTforms& tforms, ElementConfig& config, float dot_factor) const
{
	if (rocky != nullptr)
	{
		rocky->renderer.render(*rocky->server, rocky->qtree, tforms, config);
	}
	else if(gas != nullptr)
	{
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
	gas = nullptr;
}


PlanetaryBodyRenderer::~PlanetaryBodyRenderer()
{
	delete rocky;
	rocky = nullptr;

	delete atmo;
	atmo = nullptr;
}

