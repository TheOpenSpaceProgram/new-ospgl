#pragma once
#include "atmosphere/AtmosphereRenderer.h"
#include "../universe/kepler/KeplerElements.h"
#include "renderer/RockyPlanetRenderer.h"
#include "renderer/GasPlanetRenderer.h"


class SystemElement;

// A planetary body can either be a gaseous planet
// or a rocky body, both types only differ when seen from
// relatively near-by. Far away both are just a dot
class PlanetaryBodyRenderer
{
public:

	RockyPlanetRenderer* rocky;
	GasPlanetRenderer* gas;
	AtmosphereRenderer* atmo;


	void deferred(const PlanetRenderer::PlanetRenderTforms& tforms, ElementConfig& config, float dot_factor) const;

	void forward(glm::dmat4 proj_view, glm::dvec3 camera_pos,
				 ElementConfig& config, double far_plane, glm::vec3 light_dir) const;

	static void draw_debug(double t0, double t, CartesianState state, SystemElement* elem);

	PlanetaryBodyRenderer();
	~PlanetaryBodyRenderer();
};

