#pragma once
#include "../../planet_mesher/quadtree/QuadTreeDefines.h"
#include "../../planet_mesher/quadtree/QuadTreeNode.h"
#include "../../planet_mesher/mesher/PlanetTile.h"
#include "../../universe/element/body/PlanetaryBody.h"
#include "../glm/BulletGlmCompat.h"
#include <glm/glm.hpp>
#include <unordered_map>

// Handles generation of the ground shape triangles,
// and, most importantly, caching of them using the
// quadtree coordinates.
// We use a time-out based system for "forgetting" about
// tiles as this may be useful in certain situations
// such as raycasting. Every request must tell the system
// how much to wait before dumping the tile. 
// We use physics dt, so lag should not make tiles instantly 
// disappear




class GroundShapeServer
{
public:
	static constexpr size_t PHYSICS_VERT_COUNT = PlanetTile::PHYSICS_SIZE * PlanetTile::PHYSICS_SIZE;
	static constexpr size_t PHYSICS_TRI_COUNT = PHYSICS_VERT_COUNT * 3;

private:
	
	std::array<uint16_t, PlanetTile::PHYSICS_INDEX_COUNT> indices;

	struct TileAndTriangles
	{
		PlanetTilePath path;
		double time_remaining;

		btVector3 verts[PlanetTile::PHYSICS_INDEX_COUNT];

		TileAndTriangles(PlanetTilePath npath, double time, GroundShapeServer* server);
	};


public:
	
	std::unordered_map<PlanetTilePath, TileAndTriangles*, PlanetTilePathHasher> cache;

	PlanetTile::VertexArray<PlanetTileSimpleVertex, PlanetTile::PHYSICS_SIZE> work_array;
	PlanetTile::OutPhysicsArray<PlanetTile::PHYSICS_SIZE> out_array;

	sol::state lua;

	PlanetaryBody* body;

	// Needs to be called with the physics engine tick to ensure
	// proper unloading of unused tiles
	void update(double pdt);

	
	btVector3* query(QuadTreeNode* node, double time = 1.0);

	GroundShapeServer(PlanetaryBody* body);
	~GroundShapeServer();
};

