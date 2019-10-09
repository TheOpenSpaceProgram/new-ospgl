#include "PlanetTile.h"
#include "../../util/Logger.h"

void PlanetTile::generate(PlanetTilePath path)
{

	// We actually generate some more vertices than we need, so that
	// normals can smoothly interpolate between tiles

	// Note: This array does not include the skirts, but does include
	// the extra rows and columns
	std::array<PlanetTileVertex, VERTEX_COUNT> surface_verts;

	for (int y = -1; y < TILE_SIZE + 1; y++)
	{
		for (int x = -1; x < TILE_SIZE + 1; x++)
		{
			size_t r_index = (y + 1) * TILE_SIZE + (x + 1);

			PlanetTileVertex vert;
			vert.pos = glm::vec3(0.0f, 0.0f, 0.0f);

			surface_verts[r_index] = vert;
		}
	}
	
	// Generate normals (TODO)

	// Generate skirt vertices (TODO)
	std::array<PlanetTileVertex, TILE_SIZE * 4> skirts;

	// Copy actual vertices
	for (int y = 0; y < TILE_SIZE; y++)
	{
		for (int x = 0; x < TILE_SIZE; x++)
		{
			size_t o_index = (y + 1) * TILE_SIZE + (x + 1);
			size_t f_index = y * TILE_SIZE + x;

			vertices[f_index] = surface_verts[o_index];
		}
	}

	// Copy skirts
	for (size_t i = 0; i < skirts.size(); i++)
	{
		vertices[i + TILE_SIZE * TILE_SIZE] = skirts[i];
	}

}

void PlanetTile::upload()
{
	logger->check(!is_uploaded(), "Tried to upload an already uploaded tile");

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

}

PlanetTile::PlanetTile()
{
	vbo = 0;
}

PlanetTile::~PlanetTile()
{
	if (is_uploaded())
	{
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}
}
