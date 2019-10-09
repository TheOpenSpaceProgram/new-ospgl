#pragma once
#include "../mesher/PlanetTileServer.h"
#include "../quadtree/QuadTreePlanet.h"

// Handles optimized rendering of tiles, that are stored
// in a PlanetTileServer
class PlanetRenderer
{
private:

	// The index buffer is common to all tiles
	// The VAO is also shaded
	std::array<uint16_t, PlanetTile::VERTEX_COUNT * 3> indices;
	GLuint ebo, vao;

	void generate_and_upload_index_buffer();

public:

	void render(PlanetTileServer& server, QuadTreePlanet& planet, glm::mat4 projView);

	PlanetRenderer();
	~PlanetRenderer();
};

