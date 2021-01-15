#pragma once
#include <planet_mesher/quadtree/QuadTreePlanet.h>
#include <planet_mesher/mesher/PlanetTileServer.h>
#include <planet_mesher/renderer/PlanetRenderer.h>

class RockyPlanetRenderer
{
public:

	QuadTreePlanet qtree;
	PlanetTileServer* server;
	PlanetRenderer renderer;

	void load(const std::string& script, const std::string& script_path, ElementConfig& config);

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
