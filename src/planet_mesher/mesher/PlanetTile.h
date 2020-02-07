#pragma once
#include <array>
#include <glm/glm.hpp>
#include "PlanetTilePath.h"
#include <glad/glad.h>
#include <glm/gtx/normal.hpp>
#include <sol.hpp>
#include <functional>

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

struct PlanetTileSimpleVertex
{
	glm::vec3 pos;
	glm::vec3 nrm;
};


// The index buffer is common to all planet tiles
struct PlanetTile
{

	struct GeneratorInfo
	{
		glm::dvec3 coord_3d;
		glm::dvec2 coord_2d;
		double radius;
		int depth;

		bool needs_color;
	};
	
	struct GeneratorOut
	{
		double height;
		glm::dvec3 color;
	};

	bool clockwise;

	GLuint vbo, water_vbo;

	// Keep below ~128, for OpenGL reasons (index buffer too big)
	static const int TILE_SIZE = 64;
	static const int PHYSICS_SIZE = 64;
	static const int VERTEX_COUNT = TILE_SIZE * TILE_SIZE + 4;
	static const int INDEX_COUNT = (TILE_SIZE - 1) * (TILE_SIZE - 1) * 6 + (TILE_SIZE - 1) * 4 * 3;

	template <typename T, size_t S>
	using VertexArray = std::array<T, (S + 2) * (S + 2)>;

	using OutPhysicsArray = std::array<PlanetTileSimpleVertex, PHYSICS_SIZE * PHYSICS_SIZE>;

	std::array<PlanetTileVertex, VERTEX_COUNT> vertices;

	// This one is optional, so we only allocate it if needed
	std::array<PlanetTileWaterVertex, VERTEX_COUNT>* water_vertices;

	// Return true if errors happened
	bool generate(PlanetTilePath path, double planet_radius, sol::state& lua_state, bool has_water,
		VertexArray<PlanetTileVertex, PlanetTile::TILE_SIZE>* work_array);

	// Simply generates stuff to the output_array, that's it, we can be static 
	static bool generate_simple(PlanetTilePath path, double planet_radius, sol::state& lua_state,
		VertexArray<PlanetTileSimpleVertex, PlanetTile::PHYSICS_SIZE>* work_array,
		OutPhysicsArray* out_array);

	

	void upload();

	bool is_uploaded() { return vbo != 0; }

	bool has_water() { return water_vbo != 0; }


	PlanetTile();
	~PlanetTile();
};

