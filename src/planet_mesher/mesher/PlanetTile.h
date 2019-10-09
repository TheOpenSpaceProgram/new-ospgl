#pragma once
#include <array>
#include <glm/glm.hpp>
#include "PlanetTilePath.h"
#include <glad/glad.h>

// TODO: Tile vertex structure
// We may not even use colors
struct PlanetTileVertex
{
	glm::vec3 pos;
	glm::vec3 nrm;
	glm::vec3 col;
	glm::vec2 uv;
};

// The index buffer is common to all planet tiles
// TODO: A good optimization would be optional texture 
// generation (for the physics engine)
struct PlanetTile
{
	GLuint vbo;

	// Keep below ~128, for OpenGL reasons (index buffer too big)
	static const size_t TILE_SIZE = 64;
	static const size_t VERTEX_COUNT = TILE_SIZE * TILE_SIZE + TILE_SIZE * 4;

	std::array<PlanetTileVertex, VERTEX_COUNT> vertices;

	void generate(PlanetTilePath path);

	void upload();

	bool is_uploaded() { return vbo != 0; }

	PlanetTile();
	~PlanetTile();
};

