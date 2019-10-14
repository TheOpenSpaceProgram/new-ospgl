#pragma once
#include "../mesher/PlanetTileServer.h"
#include "../quadtree/QuadTreePlanet.h"
#include "../../assets/Shader.h"
// Handles optimized rendering of tiles, that are stored
// in a PlanetTileServer
// TODO: Integration with an asset manager
class PlanetRenderer
{
private:

	Shader* shader;

	// The index buffer is common to all tiles
	// The VAO is also shaded
	std::array<uint16_t, PlanetTile::VERTEX_COUNT * 6> indices;
	GLuint ebo, vao;

	// We also have a vbo for water rendering at every depth
	GLuint vbo;

	void generate_and_upload_index_buffer();

public:

	void render(PlanetTileServer& server, QuadTreePlanet& planet, glm::dmat4 proj_view_model, float far_plane);

	PlanetRenderer();
	~PlanetRenderer();
};

