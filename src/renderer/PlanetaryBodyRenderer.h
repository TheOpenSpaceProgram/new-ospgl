#pragma once
#include "../planet_mesher/quadtree/QuadTreePlanet.h"
#include "../planet_mesher/mesher/PlanetTileServer.h"
#include "../planet_mesher/renderer/PlanetRenderer.h"
#include "../atmosphere/AtmosphereRenderer.h"
#include "../universe/kepler/KeplerElements.h"

struct RockyPlanetRenderer
{
	QuadTreePlanet qtree;
	PlanetTileServer* server;
	PlanetRenderer renderer;

	void load(const std::string& script, const std::string& script_path, PlanetConfig& config);
	
	RockyPlanetRenderer()
	{
		server = nullptr;
	}

	~RockyPlanetRenderer()
	{
		if (server != nullptr)
		{
			delete server;
		}
	}
};

class SystemElement;

// A planetary body can either be a gaseous planet
// or a rocky body, both types only differ when seen from
// relatively near-by. Far away both are just a dot
class PlanetaryBodyRenderer
{
public:

	RockyPlanetRenderer* rocky;
	AtmosphereRenderer* atmo;


	void render(glm::dmat4 proj_view, glm::dmat4 model, 
		glm::dmat4 rotation_matrix, double far_plane,
		glm::dvec3 camera_pos, PlanetConfig& config, double time, glm::vec3 light_dir,
		float dot_factor);

	void draw_debug(double t, CartesianState state, SystemElement* elem);

	PlanetaryBodyRenderer();
	~PlanetaryBodyRenderer();
};

