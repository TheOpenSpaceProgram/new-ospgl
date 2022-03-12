#pragma once
#include "../mesher/PlanetTileServer.h"
#include "../quadtree/QuadTreePlanet.h"
#include "../../assets/Shader.h"
// Handles optimized rendering of tiles, that are stored
// in a PlanetTileServer
// TODO: Integration with an asset manager

#include <assets/Image.h>
class PlanetRenderer
{
private:

	AssetHandle<Image> top_tex;
	AssetHandle<Image> cliff_tex;

	Shader* shader;
	Shader* water_shader;

	// The index buffer is common to all tiles
	// The VAO is also shaded
	std::array<uint16_t, PlanetTile::INDEX_COUNT> indices;
	size_t bulk_index_count;
	GLuint ebo, vao, uv_bo;

	// Water only uses a different vao, same index buffer
	GLuint water_vao;

	void generate_and_upload_index_buffer();


	// Current detail up means which direction is up pointing
	glm::dvec3 current_detail_up;
public:

	// Camera position should be given RELATIVE to the planet
	void render(PlanetTileServer& server, QuadTreePlanet& planet, glm::dmat4 proj_view, glm::dmat4 model,
				glm::dmat4 no_rot_model, glm::dmat4 normal_matrix, float far_plane,
				glm::dvec3 camera_pos, ElementConfig& config, double time, glm::vec3 light_dir, glm::dmat4 dmodel,
				double rot);

	PlanetRenderer();
	~PlanetRenderer();
};



