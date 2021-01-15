#include "RockyPlanetRenderer.h"

void RockyPlanetRenderer::load(const std::string& script, const std::string& script_path, ElementConfig& config)
{
	delete server;
	server = new PlanetTileServer(script, script_path, &config, config.surface.has_water);
}
