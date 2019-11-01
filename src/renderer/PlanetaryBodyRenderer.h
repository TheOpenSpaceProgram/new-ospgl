#pragma once
#include "../planet_mesher/quadtree/QuadTreePlanet.h"
#include "../planet_mesher/mesher/PlanetTileServer.h"
#include "../planet_mesher/renderer/PlanetRenderer.h"
#include "../atmosphere/AtmosphereRenderer.h"

struct RockyPlanetRenderer
{
	QuadTreePlanet qtree;
	PlanetTileServer* server;
	PlanetRenderer renderer;
};

// A planetary body can either be a gaseous planet
// or a rocky body, both types only differ when seen from
// relatively near-by. Far away both are just a dot
class PlanetaryBodyRenderer
{
public:

	RockyPlanetRenderer* rocky;
	AtmosphereRenderer* atmo;

	void render(glm::dmat4 proj_view, glm::dmat4 model, float far_plane,
		glm::dvec3 camera_pos, PlanetConfig& config, double time);


	PlanetaryBodyRenderer();
	~PlanetaryBodyRenderer();
};

