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
	Shader* water_shader;

	// The index buffer is common to all tiles
	// The VAO is also shaded
	std::array<uint16_t, PlanetTile::VERTEX_COUNT * 6> indices;
	GLuint ebo, vao;

	// Water only uses a different vao, same index buffer
	GLuint water_vao;

	void generate_and_upload_index_buffer();

public:

	void render(PlanetTileServer& server, QuadTreePlanet& planet, glm::dmat4 proj_view, glm::dmat4 model, float far_plane,
		glm::dvec3 camera_pos);

	PlanetRenderer();
	~PlanetRenderer();
};

