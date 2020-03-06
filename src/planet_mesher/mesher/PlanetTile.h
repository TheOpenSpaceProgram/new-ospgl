#pragma once
#include <array>
#include <glm/glm.hpp>
#include "PlanetTilePath.h"
#include <glad/glad.h>
#include <glm/gtx/normal.hpp>
#include <sol.hpp>
#include <functional>
#include "../../lua/LuaCore.h"
#include "../../assets/AssetManager.h"

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
	static const int TILE_SIZE = 16;
	static const int PHYSICS_SIZE = TILE_SIZE / 2;
	static const int PHYSICS_GRAPHICS_RELATION = TILE_SIZE / PHYSICS_SIZE;
	static const int VERTEX_COUNT = TILE_SIZE * TILE_SIZE + 4;
	static const int INDEX_COUNT = (TILE_SIZE - 1) * (TILE_SIZE - 1) * 6 + (TILE_SIZE - 1) * 4 * 3;
	static const int PHYSICS_INDEX_COUNT = (PHYSICS_SIZE - 1) * (PHYSICS_SIZE - 1) * 6;

	template <typename T, size_t S>
	using VertexArray = std::array<T, (S + 2) * (S + 2)>;

	template<size_t S>
	using SimpleVertexArray = std::array<PlanetTileSimpleVertex, S * S>;

	std::array<PlanetTileVertex, VERTEX_COUNT> vertices;

	// This one is optional, so we only allocate it if needed
	std::array<PlanetTileWaterVertex, VERTEX_COUNT>* water_vertices;

	// Return true if errors happened
	bool generate(PlanetTilePath path, double planet_radius, sol::state& lua_state, bool has_water,
		VertexArray<PlanetTileVertex, PlanetTile::TILE_SIZE>* work_array);

	// Simply generates stuff to the output_array, that's it, we can be static 
	static bool generate_physics(PlanetTilePath path, double planet_radius, sol::state& lua_state,
		SimpleVertexArray<PHYSICS_SIZE>* work_array);

	static void prepare_lua(sol::state& lua_state);

	void upload();

	bool is_uploaded() { return vbo != 0; }

	bool has_water() { return water_vbo != 0; }

	static void generate_index_array_with_skirts(std::array<uint16_t, INDEX_COUNT>& target, size_t& bulk_index_count);

	static void generate_physics_index_array(std::array<uint16_t, PHYSICS_INDEX_COUNT>& target);


	PlanetTile();
	~PlanetTile();
};

