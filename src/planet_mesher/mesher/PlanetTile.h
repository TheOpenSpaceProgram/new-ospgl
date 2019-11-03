#pragma once
#include <array>
#include <glm/glm.hpp>
#include "PlanetTilePath.h"
#include <glad/glad.h>
#include <glm/gtx/normal.hpp>
#include <sol.hpp>

// TODO: Tile vertex structure
// We may not even use colors
struct PlanetTileVertex
{
	glm::vec3 pos;
	glm::vec3 nrm;
	glm::vec3 col;
};

struct PlanetTileWaterVertex
{
	glm::vec3 pos;
	glm::vec3 nrm;
	float depth;
};


// The index buffer is common to all planet tiles
// TODO: A good optimization would be optional texture 
// generation (for the physics engine)
struct PlanetTile
{

	bool clockwise;

	GLuint vbo, water_vbo;

	// Keep below ~128, for OpenGL reasons (index buffer too big)
	static const int TILE_SIZE = 64;
	static const int VERTEX_COUNT = TILE_SIZE * TILE_SIZE + 4;
	static const int INDEX_COUNT = (TILE_SIZE - 1) * (TILE_SIZE - 1) * 6 + (TILE_SIZE - 1) * 4 * 3;

	template <typename T>
	using VertexArray = std::array<T, (PlanetTile::TILE_SIZE + 2) * (PlanetTile::TILE_SIZE + 2)>;


	std::array<PlanetTileVertex, VERTEX_COUNT> vertices;

	// This one is optional, so we only allocate it if needed
	std::array<PlanetTileWaterVertex, VERTEX_COUNT>* water_vertices;

	// Return true if errors happened
	bool generate(PlanetTilePath path, double planet_radius, sol::state& lua_state, bool has_water,
		VertexArray<PlanetTileVertex>* work_array);

	void upload();

	bool is_uploaded() { return vbo != 0; }

	bool has_water() { return water_vbo != 0; }


	PlanetTile();
	~PlanetTile();
};

